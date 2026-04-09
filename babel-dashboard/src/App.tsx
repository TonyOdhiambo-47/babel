import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { StatusBar } from './components/StatusBar';
import { VoicePanel } from './components/VoicePanel';
import { TranslationPanel } from './components/TranslationPanel';
import { OutputPanel } from './components/OutputPanel';
import { StatePanel } from './components/StatePanel';
import { ConversationOverlay } from './components/ConversationOverlay';
import { useCompilerBridge } from './hooks/useCompilerBridge';
import { useSpeech } from './hooks/useSpeech';
import type { Backend, DashboardMode } from './types';

// The living dashboard. Four panels, one overlay, and three
// live streams hiding behind them: microphone audio going out
// to the Interpreter, interpretations coming back, and program
// output coming back from whichever backend was chosen.

export function App() {
  const bridge = useCompilerBridge();
  const speech = useSpeech();

  const [mode, setMode] = useState<DashboardMode>('live');
  const [backend, setBackend] = useState<Backend>('interpreter');
  const [overlayVisible, setOverlayVisible] = useState(false);
  const [voiceResponse, setVoiceResponse] = useState<string | null>(null);
  const lastVoiceRef = useRef('');

  // When the user finishes speaking an utterance, send it to the
  // Interpreter for translation. If the overlay is already open,
  // treat the new utterance as a reply to the clarification
  // question instead.
  const handleUtterance = useCallback(
    (text: string) => {
      lastVoiceRef.current = text;
      if (overlayVisible) {
        setVoiceResponse(text);
        return;
      }
      bridge.translate(text);
    },
    [bridge, overlayVisible],
  );

  // When a fresh interpretation comes back with yellow or red
  // slots, pop the conversation overlay up. In editor mode we
  // never show the overlay — the user is editing Babel directly.
  useEffect(() => {
    if (!bridge.interpretation) return;
    if (mode === 'editor') return;
    if (bridge.interpretation.hasYellow || bridge.interpretation.hasRed) {
      setOverlayVisible(true);
    }
  }, [bridge.interpretation, mode]);

  const handleConfirm = useCallback(
    (source: string) => {
      setOverlayVisible(false);
      setVoiceResponse(null);
      bridge.run(source, backend);
    },
    [bridge, backend],
  );

  const handleEdit = useCallback(() => {
    setOverlayVisible(false);
    setVoiceResponse(null);
    setMode('editor');
  }, []);

  const handleDismiss = useCallback(() => {
    setOverlayVisible(false);
    setVoiceResponse(null);
  }, []);

  const handleRun = useCallback(
    (source: string, chosenBackend: Backend) => {
      setBackend(chosenBackend);
      bridge.run(source, chosenBackend);
    },
    [bridge],
  );

  // Press space to toggle microphone (unless a form element is
  // focused). Keeps the demo feeling hands-free.
  useEffect(() => {
    const handler = (e: KeyboardEvent) => {
      const target = e.target as HTMLElement | null;
      const inField =
        target && (target.tagName === 'TEXTAREA' || target.tagName === 'INPUT');
      if (inField) return;
      if (e.code === 'KeyM') {
        e.preventDefault();
        if (speech.listening) speech.stop();
        else speech.start();
      }
    };
    window.addEventListener('keydown', handler);
    return () => window.removeEventListener('keydown', handler);
  }, [speech]);

  const stdoutLines = useMemo(
    () => bridge.output.filter((o) => o.stream === 'stdout').map((o) => o.text),
    [bridge.output],
  );

  const finalizedSource = useMemo(() => {
    return bridge.interpretation?.babel.trim() ?? '';
  }, [bridge.interpretation]);

  return (
    <div className={`shell ${mode}`}>
      <StatusBar
        mode={mode}
        onModeChange={setMode}
        connected={bridge.connected}
        listening={speech.listening}
      />
      <VoicePanel speech={speech} onUtterance={handleUtterance} teachingMode={mode === 'teaching'} />
      <TranslationPanel
        interpretation={bridge.interpretation}
        mode={mode}
        onRun={handleRun}
        isRunning={bridge.isRunning}
        backendSelection={backend}
        onBackendChange={setBackend}
      />
      <OutputPanel
        output={bridge.output}
        isRunning={bridge.isRunning}
        backend={bridge.lastRunBackend}
        elapsedMs={bridge.lastRunMs}
      />
      <StatePanel source={finalizedSource} output={stdoutLines} />
      <ConversationOverlay
        interpretation={bridge.interpretation}
        onConfirm={handleConfirm}
        onEdit={handleEdit}
        onDismiss={handleDismiss}
        visible={overlayVisible}
        voiceResponse={voiceResponse}
      />

      {/* A small floating affordance — the mic toggle. Stays out
          of the four-panel grid so every mode gets it. */}
      <button
        className={`mic-fab ${speech.listening ? 'on' : ''}`}
        onClick={() => (speech.listening ? speech.stop() : speech.start())}
        aria-label={speech.listening ? 'Stop listening' : 'Start listening'}
        title="Press M to toggle"
      >
        {speech.listening ? '◼ stop' : '● speak'}
      </button>
    </div>
  );
}
