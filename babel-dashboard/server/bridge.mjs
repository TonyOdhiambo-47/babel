// The bridge between the dashboard and the Babel compiler.
//
// The dashboard is a React app that knows how to draw a live view
// of a language being translated and executed. It does not know
// how to run a C binary. The Babel compiler is a C binary that
// knows how to translate and execute a language. It does not know
// how to draw.
//
// This file is the small translator that sits between them. It
// speaks WebSocket to the dashboard and shell to the compiler.
// It has two jobs:
//
//   1. Keep a long-running `./babel --json-interpret` process
//      around. Whenever the dashboard sends loose English, pipe
//      it in and stream the JSON interpretation back out.
//
//   2. When the dashboard asks to RUN a Babel program, write it
//      to a temp file, spawn `./babel`, and stream stdout / stderr
//      back line-by-line so the output panel can render it as it
//      arrives.
//
// Everything the bridge emits is a single JSON object per frame.

import { WebSocketServer } from 'ws';
import { spawn } from 'node:child_process';
import { writeFileSync, mkdtempSync } from 'node:fs';
import { join } from 'node:path';
import { tmpdir } from 'node:os';
import { fileURLToPath } from 'node:url';
import { dirname, resolve } from 'node:path';

const __dirname = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(__dirname, '..', '..');
const BABEL_BIN = resolve(REPO_ROOT, 'babel');

const PORT = Number(process.env.BRIDGE_PORT || 5174);

console.log(`[bridge] repo root:  ${REPO_ROOT}`);
console.log(`[bridge] babel bin:  ${BABEL_BIN}`);
console.log(`[bridge] listening:  ws://localhost:${PORT}`);

const wss = new WebSocketServer({ port: PORT });

function send(ws, frame) {
  if (ws.readyState === ws.OPEN) ws.send(JSON.stringify(frame));
}

// One persistent Interpreter of Tongues process per client. Spawn
// it lazily and reuse it across translation requests, so we're not
// paying process-start cost per word.
function spawnInterpreter(ws) {
  const proc = spawn(BABEL_BIN, ['--json-interpret'], {
    cwd: REPO_ROOT,
    stdio: ['pipe', 'pipe', 'pipe'],
  });

  let buffer = '';
  proc.stdout.on('data', (chunk) => {
    buffer += chunk.toString('utf8');
    let nl;
    while ((nl = buffer.indexOf('\n')) !== -1) {
      const line = buffer.slice(0, nl).trim();
      buffer = buffer.slice(nl + 1);
      if (!line) continue;
      try {
        const payload = JSON.parse(line);
        send(ws, { type: 'interpret', payload });
      } catch (err) {
        send(ws, {
          type: 'interpret_error',
          message: `could not parse interpreter line: ${line}`,
        });
      }
    }
  });

  proc.stderr.on('data', (chunk) => {
    send(ws, { type: 'bridge_log', stream: 'stderr', text: chunk.toString('utf8') });
  });

  proc.on('exit', (code) => {
    send(ws, { type: 'bridge_log', stream: 'exit', text: `interpreter exited: ${code}` });
  });

  return proc;
}

// When the dashboard asks to run Babel source, write it to a temp
// file and run ./babel (plus optional -c / -p flags) against it.
// Stream stdout/stderr back as they arrive.
function runBabel(ws, source, backend, runId) {
  const dir = mkdtempSync(join(tmpdir(), 'babel-dash-'));
  const file = join(dir, 'program.babel');
  writeFileSync(file, source, 'utf8');

  const args = [];
  const backendLabel = backend || 'interpreter';
  if (backend === 'c') args.push('-c', '-o', join(dir, 'program'));
  else if (backend === 'python') args.push('-p', '-o', join(dir, 'program.py'));
  args.push(file);

  send(ws, {
    type: 'run_start',
    runId,
    backend: backendLabel,
    source,
    timestamp: Date.now(),
  });

  const started = Date.now();
  const proc = spawn(BABEL_BIN, args, { cwd: REPO_ROOT });

  const emitLines = (chunk, stream) => {
    const text = chunk.toString('utf8');
    for (const line of text.split('\n')) {
      if (line.length === 0) continue;
      send(ws, { type: 'run_line', runId, stream, text: line });
    }
  };

  proc.stdout.on('data', (chunk) => emitLines(chunk, 'stdout'));
  proc.stderr.on('data', (chunk) => emitLines(chunk, 'stderr'));

  proc.on('error', (err) => {
    send(ws, {
      type: 'run_error',
      runId,
      message: err.message,
    });
  });

  proc.on('exit', (code) => {
    // The C backend compiles but does not execute — it leaves a
    // binary behind. If we're in C mode, run the produced binary
    // and stream its stdout, so the dashboard sees output without
    // the user needing two clicks.
    if (code === 0 && backend === 'c') {
      const bin = join(dir, 'program');
      const run = spawn(bin, [], { cwd: REPO_ROOT });
      run.stdout.on('data', (chunk) => emitLines(chunk, 'stdout'));
      run.stderr.on('data', (chunk) => emitLines(chunk, 'stderr'));
      run.on('exit', (rcode) => {
        send(ws, {
          type: 'run_done',
          runId,
          exitCode: rcode ?? 0,
          backend: backendLabel,
          elapsedMs: Date.now() - started,
        });
      });
      return;
    }
    if (code === 0 && backend === 'python') {
      const py = join(dir, 'program.py');
      const run = spawn('python3', [py], { cwd: REPO_ROOT });
      run.stdout.on('data', (chunk) => emitLines(chunk, 'stdout'));
      run.stderr.on('data', (chunk) => emitLines(chunk, 'stderr'));
      run.on('exit', (rcode) => {
        send(ws, {
          type: 'run_done',
          runId,
          exitCode: rcode ?? 0,
          backend: backendLabel,
          elapsedMs: Date.now() - started,
        });
      });
      return;
    }
    send(ws, {
      type: 'run_done',
      runId,
      exitCode: code ?? 0,
      backend: backendLabel,
      elapsedMs: Date.now() - started,
    });
  });
}

wss.on('connection', (ws) => {
  console.log('[bridge] client connected');
  const interpreter = spawnInterpreter(ws);

  send(ws, { type: 'hello', bin: BABEL_BIN, time: Date.now() });

  ws.on('message', (raw) => {
    let frame;
    try {
      frame = JSON.parse(raw.toString('utf8'));
    } catch (err) {
      send(ws, { type: 'bridge_log', stream: 'error', text: 'bad json from client' });
      return;
    }

    if (frame.type === 'translate') {
      // A line of loose English the dashboard wants translated.
      // Pipe it straight into the interpreter's stdin.
      const line = String(frame.text || '').replace(/\n/g, ' ');
      if (line.trim().length === 0) return;
      interpreter.stdin.write(line + '\n');
      return;
    }

    if (frame.type === 'run') {
      runBabel(ws, String(frame.source || ''), frame.backend || 'interpreter', frame.runId);
      return;
    }

    if (frame.type === 'ping') {
      send(ws, { type: 'pong', time: Date.now() });
      return;
    }
  });

  ws.on('close', () => {
    console.log('[bridge] client disconnected');
    try {
      interpreter.kill();
    } catch {
      /* ignore */
    }
  });
});
