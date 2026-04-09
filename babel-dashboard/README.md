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
- **Editor** — the translation panel becomes a text area. Type Babel directly.
- **Present** — only the translation and output panels, bigger fonts. For demos.
- **Teach** — every Babel statement gets an inline explanation. For Foundation Week.

## Talking to the compiler

The bridge (`server/bridge.mjs`) is the translator between the dashboard and
the Babel C binary. It keeps a long-running `./babel --json-interpret` process
around for speech-to-Babel translation, and spawns a fresh `./babel` per run.

Everything it streams back is a single JSON object per line, typed via
`src/types.ts`. If you're adding a new frame type, edit `types.ts` first, then
both ends will agree.

## Why it exists

The compiler works in a terminal. The code is on disk. The REPL is text on a
screen. All of that is invisible. This is the part people can watch.
