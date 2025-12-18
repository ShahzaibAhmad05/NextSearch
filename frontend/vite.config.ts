import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig({
  plugins: [react()],
  server: {
    // Frontend dev server (Vite) will proxy /api/* to your backend
    proxy: {
      "/api": {
        target: "http://127.0.0.1:8080",
        changeOrigin: true,
        secure: false,
        // strip /api prefix
        rewrite: (path) => path.replace(/^\/api/, ""),
      },
    },
  },
});
