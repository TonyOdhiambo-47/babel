import { useEffect, useRef, useState } from 'react';
import './ProsePanel.css';

// The left panel in Editor mode. A textarea for loose English.
// Every keystroke is debounced and handed to the Interpreter, so
// the middle panel crossfades from prose into formal Babel in
// near real time. Works for valid Babel too — the Interpreter
// passes already-correct sentences through unchanged.

interface ProsePanelProps {
  value: string;
  onChange: (text: string) => void;
  onTranslate: (text: string) => void;
}

export function ProsePanel({ value, onChange, onTranslate }: ProsePanelProps) {
  const [local, setLocal] = useState(value);
  const timer = useRef<number | null>(null);

  // Keep local state in sync if the parent ever resets the prose
  // (e.g. between runs). We only write upward on user edits.
  useEffect(() => {
    setLocal(value);
  }, [value]);

  const handleChange = (e: React.ChangeEvent<HTMLTextAreaElement>) => {
    const next = e.target.value;
    setLocal(next);
    onChange(next);
    // Debounce the translate call — a small pause after typing
    // feels more responsive than translating on every keystroke,
    // and avoids flooding the interpreter child process.
    if (timer.current) window.clearTimeout(timer.current);
    timer.current = window.setTimeout(() => {
      if (next.trim()) onTranslate(next);
    }, 250);
  };

  return (
    <section className="panel voice prose">
      <header className="panel-head">
        <span className="title">Prose</span>
        <span className="sub">loose english</span>
      </header>
      <div className="panel-body prose-body">
        <textarea
          className="prose-input"
          value={local}
          onChange={handleChange}
          placeholder={`Type however you like.\n\nmake a number called score that equals ten\nthen show me score`}
          spellCheck={false}
          autoFocus
          onKeyDown={(e) => {
            // Tab inserts four spaces — prose can still be indented
            // when people are thinking in blocks.
            if (e.key === 'Tab') {
              e.preventDefault();
              const ta = e.currentTarget;
              const { selectionStart: s, selectionEnd: en } = ta;
              const next = local.slice(0, s) + '    ' + local.slice(en);
              setLocal(next);
              onChange(next);
              if (timer.current) window.clearTimeout(timer.current);
              timer.current = window.setTimeout(() => onTranslate(next), 250);
              requestAnimationFrame(() => {
                ta.selectionStart = ta.selectionEnd = s + 4;
              });
            }
          }}
        />
      </div>
    </section>
  );
}
