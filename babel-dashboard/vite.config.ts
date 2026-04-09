import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// The dashboard talks to the Node bridge, which in turn talks to
// the Babel compiler. Vite only serves the UI; the bridge runs
// on a separate port and the dashboard connects by WebSocket.
export default defineConfig({
  plugins: [react()],
  server: {
    port: 5173,
    strictPort: true,
  },
});
