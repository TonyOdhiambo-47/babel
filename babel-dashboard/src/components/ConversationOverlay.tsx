import { useEffect, useMemo, useState } from 'react';
import type { InterpretPayload, ClarificationQuestion } from '../types';
import './ConversationOverlay.css';

// The conversation overlay. It slides up when the Interpreter of
// Tongues has yellow (or red) fragments she's unsure about, and
// asks the user to confirm in batch. In voice mode, the first
// question is also spoken aloud via speechSynthesis so the user
// can answer without looking at the screen.

interface ConversationOverlayProps {
  interpretation: InterpretPayload | null;
  onConfirm: (source: string) => void;
  onEdit: () => void;
  onDismiss: () => void;
  visible: boolean;
  voiceResponse?: string | null;
}

function questionsFor(interp: InterpretPayload): ClarificationQuestion[] {
  return interp.fragments
    .map((frag, i) => ({ frag, i }))
    .filter(({ frag }) => frag.confidence !== 'green' && frag.babel.trim().length > 0)
    .map(({ frag, i }) => ({
      id: `q_${i}`,
      fragmentIndex: i,
      question: `I heard "${frag.original.trim()}" and wrote "${frag.babel.trim()}" — does that look right?`,
      guess: frag.babel.trim(),
      accepted: null,
    }));
}

export function ConversationOverlay(props: ConversationOverlayProps) {
  const { interpretation, onConfirm, onEdit, onDismiss, visible, voiceResponse } = props;
  const questions = useMemo<ClarificationQuestion[]>(
    () => (interpretation ? questionsFor(interpretation) : []),
    [interpretation],
  );

  const [answers, setAnswers] = useState<Record<string, boolean>>({});

  // Reset answers whenever the set of questions changes.
  useEffect(() => {
    setAnswers({});
  }, [questions]);

  // When first shown, read the batch preamble aloud.
  useEffect(() => {
    if (!visible || !interpretation || questions.length === 0) return;
    if (typeof window === 'undefined' || !('speechSynthesis' in window)) return;
    const preamble =
      questions.length === 1
        ? `I built this from what you said. Does it look right?`
        : `I built this from what you said, but a few lines are guesses. Does it look right?`;
    try {
      const u = new SpeechSynthesisUtterance(preamble);
      u.rate = 1.08;
      u.pitch = 1.0;
      window.speechSynthesis.cancel();
      window.speechSynthesis.speak(u);
    } catch {
      /* ignore — tts is a nicety, not a requirement */
    }
  }, [visible, interpretation, questions]);

  // If the user answered by voice, try to parse "yes" / "no" /
  // "run it" and short-circuit accordingly.
  useEffect(() => {
    if (!voiceResponse || !visible) return;
    const lower = voiceResponse.toLowerCase();
    if (/\b(yes|yeah|yep|correct|right|run it|looks good|ship it|do it|go ahead)\b/.test(lower)) {
      if (interpretation) onConfirm(interpretation.babel);
    } else if (/\b(no|nope|wrong|edit|change|different)\b/.test(lower)) {
      onEdit();
    }
  }, [voiceResponse, visible, interpretation, onConfirm, onEdit]);

  // Keyboard shortcuts while the overlay is visible.
  useEffect(() => {
    if (!visible) return;
    const handler = (e: KeyboardEvent) => {
      if (e.key === 'Enter' || e.key === ' ') {
        e.preventDefault();
        if (interpretation) onConfirm(interpretation.babel);
      } else if (e.key === 'Escape') {
        e.preventDefault();
        onDismiss();
      }
    };
    window.addEventListener('keydown', handler);
    return () => window.removeEventListener('keydown', handler);
  }, [visible, interpretation, onConfirm, onDismiss]);

  if (!interpretation) return null;

  return (
    <div className={`conversation-overlay ${visible ? 'active' : ''}`}>
      <div className="convo-inner">
        <header className="convo-head">
          <span className="convo-title">I think this is what you mean.</span>
          <span className="convo-sub">
            {questions.length === 0
              ? 'Everything looks clean.'
              : questions.length === 1
                ? 'One line is a guess.'
                : `${questions.length} lines are guesses.`}
          </span>
        </header>

        <pre className="convo-preview">{interpretation.babel.trim()}</pre>

        {questions.length > 0 && (
          <ul className="convo-qs">
            {questions.map((q, idx) => (
              <li
                key={q.id}
                className={answers[q.id] === true ? 'yes' : answers[q.id] === false ? 'no' : ''}
                style={{ animationDelay: `${idx * 70}ms` }}
              >
                <span className="q-mark">?</span>
                <span className="q-text">{q.question}</span>
                <div className="q-choice">
                  <button
                    className={answers[q.id] === true ? 'active' : ''}
                    onClick={() => setAnswers((prev) => ({ ...prev, [q.id]: true }))}
                  >
                    yes
                  </button>
                  <button
                    className={answers[q.id] === false ? 'active' : ''}
                    onClick={() => setAnswers((prev) => ({ ...prev, [q.id]: false }))}
                  >
                    no
                  </button>
                </div>
              </li>
            ))}
          </ul>
        )}

        <div className="convo-actions">
          <button className="confirm" onClick={() => onConfirm(interpretation.babel)}>
            Yes, run it ▶
          </button>
          <button onClick={onEdit}>Edit</button>
          <button onClick={onDismiss}>Cancel</button>
          <span className="hint">
            <kbd>Space</kbd> to confirm · <kbd>Esc</kbd> to dismiss
          </span>
        </div>
      </div>
    </div>
  );
}
