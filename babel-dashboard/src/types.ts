// Shared type surface between the dashboard, the bridge, and
// the Babel compiler's --json-interpret mode.
//
// Keep this file small — the pipeline is simple, and the fewer
// intermediate shapes we invent, the less opportunity there is
// for one side to disagree with another.

export type Confidence = 'green' | 'yellow' | 'red';

/** A single fragment coming back from the Interpreter of Tongues. */
export interface InterpretFragment {
  original: string;
  babel: string;
  confidence: Confidence;
  reason: string;
}

/** One interpretation — what the Interpreter heard for one input line. */
export interface InterpretPayload {
  fragments: InterpretFragment[];
  babel: string;
  hasYellow: boolean;
  hasRed: boolean;
}

/** The backends the dashboard can choose between. */
export type Backend = 'interpreter' | 'c' | 'python';

/** The dashboard's four UI modes. */
export type DashboardMode = 'live' | 'editor' | 'presentation' | 'teaching';

/** One line of a running program's output, as it arrives. */
export interface OutputLine {
  id: number;
  runId: number;
  stream: 'stdout' | 'stderr';
  text: string;
  at: number;
}

/** The frames the bridge streams back to the client. */
export type BridgeFrame =
  | { type: 'hello'; bin: string; time: number }
  | { type: 'pong'; time: number }
  | { type: 'interpret'; payload: InterpretPayload }
  | { type: 'interpret_error'; message: string }
  | { type: 'run_start'; runId: number; backend: Backend; source: string; timestamp: number }
  | { type: 'run_line'; runId: number; stream: 'stdout' | 'stderr'; text: string }
  | { type: 'run_done'; runId: number; exitCode: number; backend: Backend; elapsedMs: number }
  | { type: 'run_error'; runId: number; message: string }
  | { type: 'bridge_log'; stream: string; text: string };

/** The frames the client sends to the bridge. */
export type ClientFrame =
  | { type: 'translate'; text: string }
  | { type: 'run'; runId: number; source: string; backend: Backend }
  | { type: 'ping' };

/** A recognised keyword in the live transcript. Mapped to the
 *  class the voice panel should paint it with. */
export type WordKind = 'keyword' | 'synonym' | 'filler' | 'unknown' | 'string' | 'number';

export interface ClassifiedWord {
  id: number;
  text: string;
  kind: WordKind;
  /** When true, this word has just arrived and should animate in. */
  fresh: boolean;
}

/** A statement detected inside the current interpretation, used by
 *  the center panel to render assembled Babel code line by line. */
export interface BabelStatement {
  id: number;
  text: string;
  /** Where this statement originated — useful for the morph. */
  sourceFragmentIndex: number;
  confidence: Confidence;
  reason: string;
}

/** A clarification question raised by the conversation overlay. */
export interface ClarificationQuestion {
  id: string;
  fragmentIndex: number;
  question: string;
  guess: string;
  accepted: boolean | null;
}

/** A declared Babel variable visible in the state panel. */
export interface StateEntry {
  name: string;
  kind: 'number' | 'word' | 'truth' | 'list' | 'recipe';
  summary: string;
  lastChanged: number;
}
