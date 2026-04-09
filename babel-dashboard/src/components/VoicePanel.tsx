import { useEffect, useMemo, useRef } from 'react';
import type { SpeechApi } from '../hooks/useSpeech';
import { classifyWord } from '../services/keywords';
import './VoicePanel.css';

// The left panel. Shows the live transcript of the user's voice,
// word by word, classified on the fly. Filler words fade to 30%.
// Recognised keywords pulse in accent colour. Recognised synonyms
// underline themselves. Everything else stays white.
//
// When the speech hook reports an "utterance complete" (1.5s
// silence), the whole transcript pulses once and is handed to
// the parent via the onUtterance prop.

interface VoicePanelProps {
  speech: SpeechApi;
  onUtterance: (text: string) => void;
  teachingMode: boolean;
}

export function VoicePanel({ speech, onUtterance, teachingMode }: VoicePanelProps) {
  const scrollRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const off = speech.onUtterance((text) => {
      onUtterance(text);
    });
    return off;
  }, [speech, onUtterance]);

  // Auto-scroll to bottom as new words arrive.
  useEffect(() => {
    if (scrollRef.current) {
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
    }
  }, [speech.final, speech.interim]);

  const words = useMemo(() => {
    const all = (speech.final + ' ' + speech.interim).trim();
    if (!all) return [];
    return all.split(/\s+/).map((w, i) => ({
      id: i,
      raw: w,
      kind: classifyWord(w),
      isInterim: i >= speech.final.trim().split(/\s+/).filter(Boolean).length,
    }));
  }, [speech.final, speech.interim]);

  return (
    <section className="panel voice">
      <header className="panel-head">
        <span className="title">Voice</span>
        <span className="sub">
          {speech.listening ? (
            <span className="rec">
              <span className="rec-dot" /> listening
            </span>
          ) : speech.supported ? (
            'idle'
          ) : (
            'unsupported'
          )}
        </span>
      </header>
      <div className="panel-body voice-body" ref={scrollRef}>
        {!speech.supported && (
          <p className="voice-unsupported">
            This browser doesn't expose a Speech Recognition API. Switch to
            <strong> Editor Mode</strong> in the top bar and type Babel directly.
          </p>
        )}
        {speech.supported && words.length === 0 && (
          <p className="voice-prompt">
            Press the microphone to start, or hit <kbd>Space</kbd>. Speak however
            you like — full sentences, half-sentences, loose English. The Interpreter
            will translate.
          </p>
        )}
        <div className="voice-stream">
          {words.map((w) => (
            <span
              key={w.id}
              className={`word word-${w.kind} ${w.isInterim ? 'interim' : ''}`}
              style={{ animationDelay: `${(w.id % 8) * 12}ms` }}
            >
              {w.raw}{' '}
            </span>
          ))}
        </div>
        {teachingMode && speech.listening && (
          <div className="voice-teach">
            <p>
              Each word you speak is being classified here. Bright teal words are
              already valid Babel. Gold underlined words are loose synonyms the
              Interpreter will rewrite. Dim words are filler she'll ignore.
            </p>
          </div>
        )}
      </div>
      {speech.error && <div className="voice-error">{speech.error}</div>}
    </section>
  );
}
