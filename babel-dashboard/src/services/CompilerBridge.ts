// A thin wrapper around the WebSocket connection to the Node
// bridge. Reconnects automatically on disconnect, with a gentle
// backoff — the dashboard is meant to be left running on a demo
// machine, so it should not need a page reload to recover.

import type { BridgeFrame, ClientFrame } from '../types';

type Listener = (frame: BridgeFrame) => void;

const BRIDGE_URL = (() => {
  const host = typeof window !== 'undefined' ? window.location.hostname : 'localhost';
  return `ws://${host || 'localhost'}:5174`;
})();

export class CompilerBridge {
  private ws: WebSocket | null = null;
  private listeners = new Set<Listener>();
  private reconnectDelay = 500;
  private shouldRun = true;
  private connectedOnce = false;
  private onConnectionChange: (connected: boolean) => void;

  constructor(onConnectionChange: (connected: boolean) => void) {
    this.onConnectionChange = onConnectionChange;
  }

  start() {
    this.shouldRun = true;
    this.open();
  }

  stop() {
    this.shouldRun = false;
    this.ws?.close();
    this.ws = null;
  }

  private open() {
    try {
      this.ws = new WebSocket(BRIDGE_URL);
    } catch (err) {
      this.scheduleReconnect();
      return;
    }

    this.ws.addEventListener('open', () => {
      this.connectedOnce = true;
      this.reconnectDelay = 500;
      this.onConnectionChange(true);
    });

    this.ws.addEventListener('message', (event) => {
      let frame: BridgeFrame;
      try {
        frame = JSON.parse(event.data) as BridgeFrame;
      } catch {
        return;
      }
      for (const listener of this.listeners) {
        try {
          listener(frame);
        } catch (err) {
          console.error('[bridge listener]', err);
        }
      }
    });

    this.ws.addEventListener('close', () => {
      this.onConnectionChange(false);
      if (this.shouldRun) this.scheduleReconnect();
    });

    this.ws.addEventListener('error', () => {
      /* the 'close' handler covers reconnect logic; swallow here */
    });
  }

  private scheduleReconnect() {
    if (!this.shouldRun) return;
    setTimeout(() => this.open(), this.reconnectDelay);
    this.reconnectDelay = Math.min(this.reconnectDelay * 1.6, 5000);
  }

  send(frame: ClientFrame) {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(frame));
    }
  }

  on(listener: Listener): () => void {
    this.listeners.add(listener);
    return () => this.listeners.delete(listener);
  }

  get url() {
    return BRIDGE_URL;
  }
}
