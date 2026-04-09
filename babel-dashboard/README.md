# The living dashboard

A browser window into the Babel compiler.

Four panels:

- **Voice** — live transcript of whatever you say, classified on the fly.
- **Translation** — loose English crossfading into formal Babel, line by line.
- **Output** — whatever the compiler prints, streaming in as it runs.
- **Memory** — every name the program declares, grouped by kind.

And one overlay — the **Conversation**, which slides up whenever the
Interpreter of Tongues is guessing and asks you to confirm.

## Running it

From the repo root:

```
make                              # build the C binary
cd babel-dashboard
npm install
npm run dev
```

That starts two processes in parallel:

1. The **Vite** dev server on `http://localhost:5173`
2. The **Node bridge** on `ws://localhost:5174`

Open the Vite URL in Chrome or Safari — the speech layer uses the browser's
built-in `SpeechRecognition` API, which isn't in Firefox yet. Allow microphone
access, press <kbd>M</kbd> (or click the `● speak` button in the corner), and
start talking.

## Four modes

- **Live** — all four panels, full pipeline visualization. The default.
- **Editor** — the voice panel is replaced by a prose pad. Type loose English
  into it and watch the Interpreter translate as you go; the microphone is
  still available if you'd rather talk.
- **Present** — only the translation and output panels, bigger fonts. For demos.
- **Teach** — every Babel statement gets an inline explanation. For Foundation Week.

## Talking to the compiler

The bridge (`server/bridge.mjs`) is the translator between the dashboard and
the Babel C binary. It has two jobs:

1. Keep one long-running `./babel --json-interpret` process around per client.
   That's a special mode of the Interpreter of Tongues: it reads one line of
   loose English from stdin at a time and writes one line of JSON to stdout,
   so the bridge doesn't pay process-start cost per word. The JSON payload
   contains the translated Babel, the kinds of slots the Interpreter is sure
   about, and any yellow/red slots that need human confirmation.

2. Spawn a fresh `./babel` (or `./babel -c` / `./babel -p`) per run, write the
   source to a temp file, and stream stdout/stderr back line by line.

Because the JSON protocol is line-based, the bridge escapes embedded newlines
in the traveler's prose to `\x01` before writing them to the interpreter's
stdin — the Interpreter un-escapes them on the other side. That matters
because paragraph breaks in the prose are how the traveler indicates dedents
in otherwise-flat text: one blank line pops one level of nesting, two pop two.

Everything the bridge streams back is a single JSON object per line, typed
via `src/types.ts`. If you're adding a new frame type, edit `types.ts` first,
then both ends will agree.

## Why it exists

The compiler works in a terminal. The code is on disk. The REPL is text on a
screen. All of that is invisible. This is the part people can watch.
