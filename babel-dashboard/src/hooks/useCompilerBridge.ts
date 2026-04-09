import { useEffect, useMemo, useRef, useState, useCallback } from 'react';
import { CompilerBridge } from '../services/CompilerBridge';
import type { BridgeFrame, InterpretPayload, OutputLine, Backend } from '../types';

// A React hook around the CompilerBridge service. It exposes the
// pieces the panels actually need:
//
//   - the latest interpretation (for the translation panel)
//   - the accumulating output lines (for the output panel)
//   - a translate() that sends loose English to the interpreter
//   - a run() that executes Babel source on a chosen backend

export interface BridgeApi {
  connected: boolean;
  interpretation: InterpretPayload | null;
  output: OutputLine[];
  isRunning: boolean;
  lastRunBackend: Backend | null;
  lastRunMs: number | null;
  translate: (text: string) => void;
  run: (source: string, backend: Backend) => void;
  clearOutput: () => void;
}

export function useCompilerBridge(): BridgeApi {
  const [connected, setConnected] = useState(false);
  const [interpretation, setInterpretation] = useState<InterpretPayload | null>(null);
  const [output, setOutput] = useState<OutputLine[]>([]);
  const [isRunning, setIsRunning] = useState(false);
  const [lastRunBackend, setLastRunBackend] = useState<Backend | null>(null);
  const [lastRunMs, setLastRunMs] = useState<number | null>(null);

  const outputIdRef = useRef(0);
  const runIdRef = useRef(0);

  const bridge = useMemo(() => new CompilerBridge(setConnected), []);

  useEffect(() => {
    const off = bridge.on((frame: BridgeFrame) => {
      switch (frame.type) {
        case 'interpret':
          setInterpretation(frame.payload);
          return;
        case 'run_start':
          setIsRunning(true);
          setLastRunBackend(frame.backend);
          setLastRunMs(null);
          return;
        case 'run_line':
          setOutput((prev) => [
            ...prev,
            {
              id: ++outputIdRef.current,
              runId: frame.runId,
              stream: frame.stream,
              text: frame.text,
              at: Date.now(),
            },
          ]);
          return;
        case 'run_done':
          setIsRunning(false);
          setLastRunMs(frame.elapsedMs);
          return;
        case 'run_error':
          setIsRunning(false);
          setOutput((prev) => [
            ...prev,
            {
              id: ++outputIdRef.current,
              runId: frame.runId,
              stream: 'stderr',
              text: frame.message,
              at: Date.now(),
            },
          ]);
          return;
      }
    });
    bridge.start();
    return () => {
      off();
      bridge.stop();
    };
  }, [bridge]);

  const translate = useCallback(
    (text: string) => {
      if (!text.trim()) return;
      bridge.send({ type: 'translate', text });
    },
    [bridge],
  );

  const run = useCallback(
    (source: string, backend: Backend) => {
      if (!source.trim()) return;
      const runId = ++runIdRef.current;
      setOutput((prev) => prev.filter((line) => line.runId === runId));
      bridge.send({ type: 'run', runId, source, backend });
    },
    [bridge],
  );

  const clearOutput = useCallback(() => setOutput([]), []);

  return {
    connected,
    interpretation,
    output,
    isRunning,
    lastRunBackend,
    lastRunMs,
    translate,
    run,
    clearOutput,
  };
}
