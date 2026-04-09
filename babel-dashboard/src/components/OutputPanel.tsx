import { useEffect, useRef } from 'react';
import type { OutputLine, Backend } from '../types';
import './OutputPanel.css';

// The right panel. Shows stdout and stderr from the compiler run
// as they arrive. Lines fade in one at a time. Numeric outputs
// briefly flash the "success" accent. stderr lines come through
// in amber. A small backend chip + elapsed time sits above.

interface OutputPanelProps {
  output: OutputLine[];
  isRunning: boolean;
  backend: Backend | null;
  elapsedMs: number | null;
}

export function OutputPanel({ output, isRunning, backend, elapsedMs }: OutputPanelProps) {
  const scrollRef = useRef<HTMLDivElement>(null);
  useEffect(() => {
    if (scrollRef.current) scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
  }, [output]);

  return (
    <section className="panel output">
      <header className="panel-head">
        <span className="title">Output</span>
        <div className="out-status">
          {backend && (
            <span className={`chip small ${isRunning ? 'warn' : 'ok'}`}>
              <span className="dot" />
              {backend}
            </span>
          )}
          {!isRunning && elapsedMs !== null && (
            <span className="chip small">
              <span className="dot" />
              {elapsedMs < 1000 ? `${elapsedMs}ms` : `${(elapsedMs / 1000).toFixed(2)}s`}
            </span>
          )}
        </div>
      </header>
      <div className="panel-body output-body" ref={scrollRef}>
        {output.length === 0 && !isRunning && (
          <p className="out-hint">Output will appear here as the program runs.</p>
        )}
        {isRunning && output.length === 0 && (
          <p className="out-running">→ running…</p>
        )}
        {output.map((line) => (
          <div key={line.id} className={`out-line stream-${line.stream}`}>
            {line.stream === 'stderr' && <span className="out-arrow">!</span>}
            <span className="out-text">{line.text}</span>
          </div>
        ))}
      </div>
    </section>
  );
}
