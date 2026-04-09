import { useMemo } from 'react';
import type { StateEntry } from '../types';
import './StatePanel.css';

// The bottom panel. Shows the variables, lists, and recipes the
// current Babel source declares.
//
// Babel doesn't (yet) expose a runtime state API, so this panel
// is a static parse of the finalized Babel source — it tells you
// what names the program *will* bring into existence, not what
// their current values are. When the runtime one day emits a
// trace, the parser-driven view gets replaced line for line.

interface StatePanelProps {
  source: string;
  output: string[];
}

const DECL_RE = /Let there be a (number|word|truth|list) called "([^"]+)"(?:\s+that\s+(equals|begins)\s+([^.]+))?/gi;
const RECIPE_RE = /^To (\w+)(?:\s+with\s+([^,]+))?,\s+do the following:/gim;

function parseEntries(source: string): StateEntry[] {
  const entries: StateEntry[] = [];
  const seen = new Set<string>();

  DECL_RE.lastIndex = 0;
  let m: RegExpExecArray | null;
  while ((m = DECL_RE.exec(source)) !== null) {
    const kind = m[1].toLowerCase() as StateEntry['kind'];
    const name = m[2];
    if (seen.has(name)) continue;
    seen.add(name);
    const summary = m[4] ? m[4].trim().replace(/\s+/g, ' ') : '—';
    entries.push({ name, kind, summary, lastChanged: Date.now() });
  }

  RECIPE_RE.lastIndex = 0;
  while ((m = RECIPE_RE.exec(source)) !== null) {
    const name = m[1];
    const args = m[2] ? m[2].replace(/\s+and\s+/gi, ', ').trim() : '';
    if (seen.has(name)) continue;
    seen.add(name);
    entries.push({
      name,
      kind: 'recipe',
      summary: args ? `with ${args}` : 'no arguments',
      lastChanged: Date.now(),
    });
  }

  return entries;
}

export function StatePanel({ source, output }: StatePanelProps) {
  const entries = useMemo(() => parseEntries(source), [source]);
  const printsSeen = output.length;

  return (
    <section className="panel state">
      <header className="panel-head">
        <span className="title">Memory</span>
        <span className="sub">{entries.length} names · {printsSeen} lines printed</span>
      </header>
      <div className="panel-body state-body">
        {entries.length === 0 && (
          <p className="state-empty">
            When the program declares a number, word, list, truth, or recipe, it'll show
            up here.
          </p>
        )}
        <div className="state-grid">
          {entries.map((entry) => (
            <article key={entry.name} className={`state-card kind-${entry.kind}`}>
              <header>
                <span className="state-kind">{entry.kind}</span>
                <span className="state-name">{entry.name}</span>
              </header>
              <p className="state-summary">{entry.summary}</p>
            </article>
          ))}
        </div>
      </div>
    </section>
  );
}
