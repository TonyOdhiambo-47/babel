import { useEffect, useRef, useState, useCallback } from 'react';

// The speech-to-text layer, abstracted behind a single hook so we
// can swap the engine later. For the MVP it uses the browser's
// built-in SpeechRecognition API, which gives us streaming
// partials, interim results, and final utterances with no
// download. It's Chrome/Edge/Safari only — Firefox users see a
// textarea instead, which is a reasonable fallback.
//
// When the user stops speaking for ~1.5s, we emit an
// "utteranceComplete" event with the final transcript, so the
// conversation overlay can wake up if needed.

type SpeechRecognitionLike = {
  continuous: boolean;
  interimResults: boolean;
  lang: string;
  maxAlternatives?: number;
  onresult: ((event: any) => void) | null;
  onerror: ((event: any) => void) | null;
  onend: (() => void) | null;
  start: () => void;
  stop: () => void;
};

type SpeechRecognitionCtor = new () => SpeechRecognitionLike;

function getSpeechCtor(): SpeechRecognitionCtor | null {
  if (typeof window === 'undefined') return null;
  const w = window as any;
  return w.SpeechRecognition || w.webkitSpeechRecognition || null;
}

export interface SpeechApi {
  supported: boolean;
  listening: boolean;
  interim: string;
  final: string;
  error: string | null;
  start: () => void;
  stop: () => void;
  /** Clear the final transcript without stopping the recogniser. */
  reset: () => void;
  /** Subscribe to "utterance complete" events (silence detected). */
  onUtterance: (handler: (text: string) => void) => () => void;
}

export function useSpeech(): SpeechApi {
  const ctor = getSpeechCtor();
  const supported = ctor !== null;

  const [listening, setListening] = useState(false);
  const [interim, setInterim] = useState('');
  const [finalText, setFinalText] = useState('');
  const [error, setError] = useState<string | null>(null);

  const recRef = useRef<SpeechRecognitionLike | null>(null);
  const silenceTimerRef = useRef<number | null>(null);
  const listenersRef = useRef(new Set<(text: string) => void>());
  const pendingRef = useRef('');
  /* If the recogniser ends on its own (Chrome auto-stops after
   * ~60s of silence), we want to restart it as long as the user
   * hasn't explicitly pressed stop. */
  const wantRunningRef = useRef(false);

  const scheduleSilence = useCallback(() => {
    if (silenceTimerRef.current) window.clearTimeout(silenceTimerRef.current);
    silenceTimerRef.current = window.setTimeout(() => {
      const text = pendingRef.current.trim();
      if (text.length > 0) {
        for (const l of listenersRef.current) l(text);
        pendingRef.current = '';
      }
    }, 800);
  }, []);

  const start = useCallback(() => {
    if (!supported || recRef.current) return;
    const Rec = ctor!;
    const rec = new Rec();
    rec.continuous = true;
    rec.interimResults = true;
    rec.lang = 'en-US';
    rec.maxAlternatives = 1;
    rec.onresult = (event: any) => {
      let interimPart = '';
      let finalPart = '';
      for (let i = event.resultIndex; i < event.results.length; i++) {
        const result = event.results[i];
        const transcript = result[0].transcript;
        if (result.isFinal) {
          finalPart += transcript + ' ';
        } else {
          interimPart += transcript;
        }
      }
      setInterim(interimPart);
      if (finalPart.length > 0) {
        setFinalText((prev) => (prev + ' ' + finalPart).trim());
        pendingRef.current = (pendingRef.current + ' ' + finalPart).trim();
      }
      scheduleSilence();
    };
    rec.onerror = (event: any) => {
      setError(event?.error ? String(event.error) : 'speech error');
    };
    rec.onend = () => {
      recRef.current = null;
      if (wantRunningRef.current) {
        /* Chrome sometimes ends after a long silence. Restart
         * transparently so the mic stays "open" until the user
         * presses stop. */
        try {
          const next = new Rec();
          next.continuous = true;
          next.interimResults = true;
          next.lang = 'en-US';
          next.maxAlternatives = 1;
          next.onresult = rec.onresult;
          next.onerror = rec.onerror;
          next.onend = rec.onend;
          next.start();
          recRef.current = next;
          return;
        } catch {
          /* fall through to "listening false" below */
        }
      }
      setListening(false);
    };
    try {
      wantRunningRef.current = true;
      rec.start();
      recRef.current = rec;
      setListening(true);
      setError(null);
    } catch (err: any) {
      setError(err?.message || 'start failed');
    }
  }, [ctor, supported, scheduleSilence]);

  const stop = useCallback(() => {
    wantRunningRef.current = false;
    if (silenceTimerRef.current) {
      window.clearTimeout(silenceTimerRef.current);
      silenceTimerRef.current = null;
    }
    recRef.current?.stop();
    recRef.current = null;
    setListening(false);
  }, []);

  const reset = useCallback(() => {
    setFinalText('');
    setInterim('');
    pendingRef.current = '';
  }, []);

  const onUtterance = useCallback((handler: (text: string) => void) => {
    listenersRef.current.add(handler);
    return () => {
      listenersRef.current.delete(handler);
    };
  }, []);

  useEffect(() => {
    return () => stop();
  }, [stop]);

  return { supported, listening, interim, final: finalText, error, start, stop, reset, onUtterance };
}
