import { useEffect, useMemo, useRef, useState } from 'react';
import type { InterpretPayload, Backend, DashboardMode } from '../types';
import './TranslationPanel.css';

// The middle panel. Shows the Interpreter's verdict. For each
// fragment of what the user said, it shows the original words
// above a crossfade to the final Babel translation. New lines
// slide in with a subtle staggered reveal. Once the lines settle,
// the panel offers a Run button with a backend selector.
//
// In Editor Mode the whole panel becomes a textarea — the user
// types Babel directly and presses Run. In Teaching Mode each
// statement gets a small tooltip explaining what it does.

interface TranslationPanelProps {
  interpretation: InterpretPayload | null;
  mode: DashboardMode;
  onRun: (source: string, backend: Backend) => void;
  isRunning: boolean;
  backendSelection: Backend;
  onBackendChange: (backend: Backend) => void;
}

const SYNTAX_RE = /("[^"]*"|\b\d+(?:\.\d+)?\b|\b(?:Let|there|be|a|an|called|that|equals|begins|empty|Set|Change|to|Remember|as|it|our|Print|Say|separated|by|with|If|Otherwise|is|not|and|greater|less|than|divisible|contains|in|For|every|from|While|do|the|following|Stop|To|answer|give|back|of|length|sum|true|false|plus|minus|times|divided|modulo|number|word|truth|list)\b)/g;

function highlight(line: string): React.ReactNode {
  const parts: React.ReactNode[] = [];
  let last = 0;
  let match: RegExpExecArray | null;
  SYNTAX_RE.lastIndex = 0;
  while ((match = SYNTAX_RE.exec(line)) !== null) {
    if (match.index > last) parts.push(line.slice(last, match.index));
    const tok = match[0];
    if (tok.startsWith('"')) {
      parts.push(
        <span key={`s-${match.index}`} className="tok-str">
          {tok}
        </span>,
      );
    } else if (/^\d/.test(tok)) {
      parts.push(
        <span key={`n-${match.index}`} className="tok-num">
          {tok}
        </span>,
      );
    } else {
      parts.push(
        <span key={`k-${match.index}`} className="tok-kw">
          {tok}
        </span>,
      );
    }
    last = match.index + tok.length;
  }
  if (last < line.length) parts.push(line.slice(last));
  return parts;
}

const TEACH_NOTES: Array<{ match: RegExp; note: string }> = [
  { match: /^Let there be/i, note: 'Creates a new named value. The tower will keep it alive until the end of the program.' },
  { match: /^For every .+ in /i, note: 'Walks a list. The name before "in" becomes each element in turn.' },
  { match: /^For every .+ from /i, note: 'A counted loop. Inclusive at both ends — from 1 to 5 visits 1, 2, 3, 4, 5.' },
  { match: /^If .*do the following/i, note: 'A conditional with a block body. Several statements run only if the condition holds.' },
  { match: /^If /i, note: 'A conditional with a single-statement body. If the condition holds, the rest of the line runs.' },
  { match: /^While /i, note: 'Repeats the indented block as long as the condition holds.' },
  { match: /^Print /i, note: 'Speaks a value aloud. Numbers come out as numbers, lists come out separated by commas.' },
  { match: /^Say /i, note: 'Prints a single value with no list separator handling. Use it for plain strings.' },
  { match: /^Remember /i, note: 'Appends a value to the end of a list.' },
  { match: /^Set /i, note: 'Changes an existing named value.' },
  { match: /^To /i, note: 'Defines a recipe — Babel\'s word for a function. The indented block is its body.' },
];

function teachNote(line: string): string | null {
  for (const entry of TEACH_NOTES) if (entry.match.test(line.trim())) return entry.note;
  return null;
}

export function TranslationPanel(props: TranslationPanelProps) {
  const { interpretation, mode, onRun, isRunning, backendSelection, onBackendChange } = props;

  // In editor mode the textarea is the source of truth. In every
  // other mode the interpreter's reconstructed Babel is.
  const [draft, setDraft] = useState('');
  useEffect(() => {
    if (mode !== 'editor' && interpretation) {
      setDraft(interpretation.babel.trim());
    }
  }, [interpretation, mode]);

  const lines = useMemo(() => {
    const source = mode === 'editor' ? draft : interpretation?.babel ?? '';
    return source
      .split('\n')
      .map((line, i) => ({ id: i, text: line, isBlank: line.trim().length === 0 }));
  }, [interpretation, draft, mode]);

  const revealKey = useMemo(() => JSON.stringify(lines.map((l) => l.text)), [lines]);

  const handleRun = () => {
    const src = mode === 'editor' ? draft : interpretation?.babel ?? '';
    if (src.trim()) onRun(src, backendSelection);
  };

  return (
    <section className="panel center">
      <header className="panel-head">
        <span className="title">Translation</span>
        <div className="panel-head-right">
          <div className="backend-picker">
            {(['interpreter', 'c', 'python'] as Backend[]).map((b) => (
              <button
                key={b}
                className={b === backendSelection ? 'active' : ''}
                onClick={() => onBackendChange(b)}
              >
                {b === 'interpreter' ? 'interp' : b}
              </button>
            ))}
          </div>
          <button
            className="run-btn"
            onClick={handleRun}
            disabled={isRunning || (!interpretation && !draft.trim())}
          >
            {isRunning ? 'running…' : 'run ▶'}
          </button>
        </div>
      </header>
      <div className="panel-body translation-body" key={revealKey}>
        {mode === 'editor' && (
          <textarea
            className="editor"
            value={draft}
            onChange={(e) => setDraft(e.target.value)}
            placeholder="Let there be a number called &quot;score&quot; that equals 0.
Print score."
            spellCheck={false}
          />
        )}
        {mode !== 'editor' && !interpretation && (
          <div className="translation-empty">
            <p className="small-quote">“Speak, and the tower will answer.”</p>
            <p className="hint">
              Loose English on the left becomes formal Babel here. Watch the translation
              happen as you talk.
            </p>
          </div>
        )}
        {mode !== 'editor' && interpretation && (
          <div className="assembly">
            {lines.map((line, idx) =>
              line.isBlank ? (
                <div key={line.id} className="asm-blank" />
              ) : (
                <div
                  key={line.id}
                  className="asm-line"
                  style={{ animationDelay: `${idx * 60}ms` }}
                >
                  <pre className="asm-code">{highlight(line.text)}</pre>
                  {mode === 'teaching' && teachNote(line.text) && (
                    <div className="teach-tooltip">{teachNote(line.text)}</div>
                  )}
                </div>
              ),
            )}
            {interpretation.hasRed && (
              <div className="asm-warn">
                Something in there didn't make it through the translation. Check the
                conversation overlay.
              </div>
            )}
          </div>
        )}
      </div>
    </section>
  );
}
