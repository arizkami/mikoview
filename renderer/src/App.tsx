import { APITester } from "./APITester";
import "./index.css";
import { Globe, Rocket, Gamepad2, Code, Zap, Monitor } from "lucide-react";

export function App() {
  return (
    <div className="min-h-screen bg-black">
      <div className="max-w-5xl mx-auto px-8 py-12">
        {/* Header */}
        <div className="text-center mb-16">
          <div className="flex justify-center items-center gap-4 mb-8">
            <div className="w-16 h-16 bg-white rounded-xl flex items-center justify-center">
              <Code className="w-8 h-8 text-black" />
            </div>
            <h1 className="text-4xl font-bold text-white">SwipeWebview</h1>
          </div>
          <p className="text-xl text-gray-400 max-w-2xl mx-auto leading-relaxed">
            A powerful desktop application built with modern web technologies and native performance
          </p>
        </div>

        {/* Tech Stack Cards */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-16">
          <div className="group p-6 rounded-xl border border-gray-800 bg-gray-900/50 hover:border-gray-700 transition-all duration-200">
            <div className="w-12 h-12 bg-blue-500/10 rounded-lg flex items-center justify-center mb-4 group-hover:bg-blue-500/20 transition-colors">
              <Globe className="w-6 h-6 text-blue-400" />
            </div>
            <h3 className="text-lg font-semibold text-white mb-2">React Frontend</h3>
            <p className="text-gray-400 text-sm">Modern UI with hot reloading and component-based architecture</p>
          </div>
          
          <div className="group p-6 rounded-xl border border-gray-800 bg-gray-900/50 hover:border-gray-700 transition-all duration-200">
            <div className="w-12 h-12 bg-purple-500/10 rounded-lg flex items-center justify-center mb-4 group-hover:bg-purple-500/20 transition-colors">
              <Rocket className="w-6 h-6 text-purple-400" />
            </div>
            <h3 className="text-lg font-semibold text-white mb-2">CEF Integration</h3>
            <p className="text-gray-400 text-sm">Chromium Embedded Framework for web content rendering</p>
          </div>
          
          <div className="group p-6 rounded-xl border border-gray-800 bg-gray-900/50 hover:border-gray-700 transition-all duration-200">
            <div className="w-12 h-12 bg-green-500/10 rounded-lg flex items-center justify-center mb-4 group-hover:bg-green-500/20 transition-colors">
              <Gamepad2 className="w-6 h-6 text-green-400" />
            </div>
            <h3 className="text-lg font-semibold text-white mb-2">SDL Window</h3>
            <p className="text-gray-400 text-sm">Cross-platform window management and event handling</p>
          </div>
        </div>

        {/* Get Started Section */}
        <div className="text-center mb-16">
          <div className="inline-flex items-center gap-2 px-4 py-2 bg-gray-800 rounded-full text-sm text-gray-300 mb-6">
            <Zap className="w-4 h-4" />
            Get started by editing
          </div>
          <div className="bg-gray-900 border border-gray-800 rounded-lg p-4 max-w-md mx-auto">
            <code className="text-blue-400 font-mono text-sm">src/App.tsx</code>
          </div>
          <p className="text-gray-500 text-sm mt-3">Save to see your changes instantly</p>
        </div>

        {/* Features Grid */}
        <div className="mb-16">
          <h2 className="text-2xl font-bold text-white text-center mb-8">Built for Performance</h2>
          <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
            <div className="flex gap-4">
              <div className="w-10 h-10 bg-blue-500/10 rounded-lg flex items-center justify-center flex-shrink-0">
                <Zap className="w-5 h-5 text-blue-400" />
              </div>
              <div>
                <h3 className="text-white font-semibold mb-1">Fast Refresh</h3>
                <p className="text-gray-400 text-sm">Experience instant feedback with hot module replacement</p>
              </div>
            </div>
            
            <div className="flex gap-4">
              <div className="w-10 h-10 bg-purple-500/10 rounded-lg flex items-center justify-center flex-shrink-0">
                <Monitor className="w-5 h-5 text-purple-400" />
              </div>
              <div>
                <h3 className="text-white font-semibold mb-1">Native Performance</h3>
                <p className="text-gray-400 text-sm">Combine web flexibility with desktop-class performance</p>
              </div>
            </div>
            
            <div className="flex gap-4">
              <div className="w-10 h-10 bg-green-500/10 rounded-lg flex items-center justify-center flex-shrink-0">
                <Globe className="w-5 h-5 text-green-400" />
              </div>
              <div>
                <h3 className="text-white font-semibold mb-1">Cross Platform</h3>
                <p className="text-gray-400 text-sm">Deploy to Windows, macOS, and Linux with one codebase</p>
              </div>
            </div>
            
            <div className="flex gap-4">
              <div className="w-10 h-10 bg-orange-500/10 rounded-lg flex items-center justify-center flex-shrink-0">
                <Code className="w-5 h-5 text-orange-400" />
              </div>
              <div>
                <h3 className="text-white font-semibold mb-1">Modern Stack</h3>
                <p className="text-gray-400 text-sm">Built with the latest React, TypeScript, and Tailwind CSS</p>
              </div>
            </div>
          </div>
        </div>

        {/* API Tester Section */}
        <div className="border border-gray-800 rounded-xl bg-gray-900/30 p-8">
          <div className="text-center mb-6">
            <h2 className="text-xl font-semibold text-white mb-2">API Testing</h2>
            <p className="text-gray-400">Test your API endpoints directly from the interface</p>
          </div>
          <APITester />
        </div>

        {/* Footer */}
        <footer className="text-center mt-16 pt-8 border-t border-gray-800">
          <p className="text-gray-500 text-sm">
            Built with ❤️ using React, CEF, and SDL
          </p>
        </footer>
      </div>
    </div>
  );
}

export default App;