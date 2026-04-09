import type { DashboardMode } from '../types';

// The top strip. Brand + tagline + mode selector + connection
// indicator. Small enough to stay out of the way, but visible at
// a glance — so the audience always knows which mode the demo
// is in.

interface StatusBarProps {
  mode: DashboardMode;
  onModeChange: (mode: DashboardMode) => void;
  connected: boolean;
  listening: boolean;
}

const MODES: Array<{ id: DashboardMode; label: string }> = [
  { id: 'live', label: 'Live' },
  { id: 'editor', label: 'Editor' },
  { id: 'presentation', label: 'Present' },
  { id: 'teaching', label: 'Teach' },
];

export function StatusBar({ mode, onModeChange, connected, listening }: StatusBarProps) {
  return (
    <header className="statusbar">
      <div className="brand">Babel</div>
      <span className="tagline">a window into the mind of a compiler</span>
      <span className="spacer" />
      <div className="mode-switch">
        {MODES.map((m) => (
          <button
            key={m.id}
            className={m.id === mode ? 'active' : ''}
            onClick={() => onModeChange(m.id)}
          >
            {m.label}
          </button>
        ))}
      </div>
      <span className={`chip ${connected ? 'ok' : 'bad'}`}>
        <span className="dot" />
        {connected ? 'bridge' : 'offline'}
      </span>
      <span className={`chip ${listening ? 'ok' : ''}`}>
        <span className="dot" />
        {listening ? 'mic' : 'mute'}
      </span>
    </header>
  );
}
