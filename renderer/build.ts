#!/usr/bin/env bun
import { build, type BuildConfig } from "bun";
import plugin from "bun-plugin-tailwind";
import { existsSync, readFileSync, writeFileSync } from "fs";
import { rm } from "fs/promises";
import path from "path";

// Print help text if requested
if (process.argv.includes("--help") || process.argv.includes("-h")) {
  console.log(`
🏗️  Bun Build Script

Usage: bun run build.ts [options]

Common Options:
  --outdir <path>          Output directory (default: "dist")
  --singlefile             Bundle CSS, JS, and assets into single HTML file
  --minify                 Enable minification (or --minify.whitespace, --minify.syntax, etc)
  --source-map <type>      Sourcemap type: none|linked|inline|external
  --target <target>        Build target: browser|bun|node
  --format <format>        Output format: esm|cjs|iife
  --splitting              Enable code splitting
  --packages <type>        Package handling: bundle|external
  --public-path <path>     Public path for assets
  --env <mode>             Environment handling: inline|disable|prefix*
  --conditions <list>      Package.json export conditions (comma separated)
  --external <list>        External packages (comma separated)
  --banner <text>          Add banner text to output
  --footer <text>          Add footer text to output
  --define <obj>           Define global constants (e.g. --define.VERSION=1.0.0)
  --help, -h               Show this help message

Example:
  bun run build.ts --outdir=dist --minify --source-map=linked --external=react,react-dom
  bun run build.ts --singlefile --minify
`);
  process.exit(0);
}

// Helper function to convert kebab-case to camelCase
const toCamelCase = (str: string): string => {
  return str.replace(/-([a-z])/g, g => g[1].toUpperCase());
};

// Helper function to parse a value into appropriate type
const parseValue = (value: string): any => {
  // Handle true/false strings
  if (value === "true") return true;
  if (value === "false") return false;

  // Handle numbers
  if (/^\d+$/.test(value)) return parseInt(value, 10);
  if (/^\d*\.\d+$/.test(value)) return parseFloat(value);

  // Handle arrays (comma-separated)
  if (value.includes(",")) return value.split(",").map(v => v.trim());

  // Default to string
  return value;
};

// Magical argument parser that converts CLI args to BuildConfig
function parseArgs(): Partial<BuildConfig> {
  const config: Record<string, any> = {};
  const args = process.argv.slice(2);

  for (let i = 0; i < args.length; i++) {
    const arg = args[i];
    if (!arg.startsWith("--")) continue;

    // Handle --no-* flags
    if (arg.startsWith("--no-")) {
      const key = toCamelCase(arg.slice(5));
      config[key] = false;
      continue;
    }

    // Handle --flag (boolean true)
    if (!arg.includes("=") && (i === args.length - 1 || args[i + 1].startsWith("--"))) {
      const key = toCamelCase(arg.slice(2));
      config[key] = true;
      continue;
    }

    // Handle --key=value or --key value
    let key: string;
    let value: string;

    if (arg.includes("=")) {
      [key, value] = arg.slice(2).split("=", 2);
    } else {
      key = arg.slice(2);
      value = args[++i];
    }

    // Convert kebab-case key to camelCase
    key = toCamelCase(key);

    // Handle nested properties (e.g. --minify.whitespace)
    if (key.includes(".")) {
      const [parentKey, childKey] = key.split(".");
      config[parentKey] = config[parentKey] || {};
      config[parentKey][childKey] = parseValue(value);
    } else {
      config[key] = parseValue(value);
    }
  }

  return config as Partial<BuildConfig>;
}

// Helper function to format file sizes
const formatFileSize = (bytes: number): string => {
  const units = ["B", "KB", "MB", "GB"];
  let size = bytes;
  let unitIndex = 0;

  while (size >= 1024 && unitIndex < units.length - 1) {
    size /= 1024;
    unitIndex++;
  }

  return `${size.toFixed(2)} ${units[unitIndex]}`;
};

// Helper function to inline assets into HTML
const inlineAssets = async (htmlPath: string, outputs: any[]): Promise<void> => {
  let htmlContent = readFileSync(htmlPath, 'utf-8');
  const htmlDir = path.dirname(htmlPath);
  
  // Find corresponding JS and CSS files
  const jsFiles = outputs.filter(output => output.kind === 'entry-point' && output.path.endsWith('.js'));
  const cssFiles = outputs.filter(output => output.kind === 'asset' && output.path.endsWith('.css'));
  
  // Inline CSS files
  for (const cssFile of cssFiles) {
    const cssContent = readFileSync(cssFile.path, 'utf-8');
    const cssFileName = path.basename(cssFile.path);
    
    // Replace CSS link tags with inline styles
    const cssLinkRegex = new RegExp(`<link[^>]*href=["']([^"']*${cssFileName})["'][^>]*>`, 'gi');
    htmlContent = htmlContent.replace(cssLinkRegex, `<style>${cssContent}</style>`);
  }
  
  // Inline JS files
  for (const jsFile of jsFiles) {
    const jsContent = readFileSync(jsFile.path, 'utf-8');
    const jsFileName = path.basename(jsFile.path);
    
    // Replace script src tags with inline scripts
    const scriptSrcRegex = new RegExp(`<script[^>]*src=["']([^"']*${jsFileName})["'][^>]*></script>`, 'gi');
    htmlContent = htmlContent.replace(scriptSrcRegex, `<script>${jsContent}</script>`);
  }
  
  // Handle other assets (images, fonts, etc.) by converting to data URLs
  const assetFiles = outputs.filter(output => 
    output.kind === 'asset' && 
    !output.path.endsWith('.css') && 
    !output.path.endsWith('.js') &&
    !output.path.endsWith('.html')
  );
  
  for (const assetFile of assetFiles) {
    const assetContent = readFileSync(assetFile.path);
    const assetFileName = path.basename(assetFile.path);
    const ext = path.extname(assetFile.path).toLowerCase();
    
    // Determine MIME type
    let mimeType = 'application/octet-stream';
    if (ext === '.png') mimeType = 'image/png';
    else if (ext === '.jpg' || ext === '.jpeg') mimeType = 'image/jpeg';
    else if (ext === '.gif') mimeType = 'image/gif';
    else if (ext === '.svg') mimeType = 'image/svg+xml';
    else if (ext === '.woff') mimeType = 'font/woff';
    else if (ext === '.woff2') mimeType = 'font/woff2';
    else if (ext === '.ttf') mimeType = 'font/ttf';
    
    const dataUrl = `data:${mimeType};base64,${assetContent.toString('base64')}`;
    
    // Replace asset references with data URLs
    const assetRegex = new RegExp(`(["'])([^"']*${assetFileName})(["'])`, 'gi');
    htmlContent = htmlContent.replace(assetRegex, `$1${dataUrl}$3`);
  }
  
  // Write the modified HTML back
  writeFileSync(htmlPath, htmlContent, 'utf-8');
};

console.log("\n🚀 Starting build process...\n");

// Parse CLI arguments with our magical parser
const cliConfig = parseArgs();
const outdir = cliConfig.outdir || path.join(process.cwd(), "dist");
const singleFile = (cliConfig as any).singlefile || false;

if (existsSync(outdir)) {
  console.log(`🗑️ Cleaning previous build at ${outdir}`);
  await rm(outdir, { recursive: true, force: true });
}

const start = performance.now();

// Scan for all HTML files in the project
const entrypoints = [...new Bun.Glob("**.html").scanSync("src")]
  .map(a => path.resolve("src", a))
  .filter(dir => !dir.includes("node_modules"));
console.log(`📄 Found ${entrypoints.length} HTML ${entrypoints.length === 1 ? "file" : "files"} to process\n`);

// Build configuration
const buildConfig: BuildConfig = {
  entrypoints,
  outdir,
  plugins: [plugin],
  minify: true,
  target: "browser",
  sourcemap: singleFile ? "none" : "linked", // Disable sourcemaps for single file builds
  define: {
    "process.env.NODE_ENV": JSON.stringify("production"),
  },
  ...cliConfig, // Merge in any CLI-provided options
};

// Remove singlefile from build config as it's not a Bun build option
delete (buildConfig as any).singlefile;

// Build all the HTML files
const result = await build(buildConfig);

// If singlefile mode is enabled, inline all assets
if (singleFile) {
  console.log("📦 Inlining assets into HTML files...");
  
  const htmlFiles = result.outputs.filter(output => output.path.endsWith('.html'));
  
  for (const htmlFile of htmlFiles) {
    await inlineAssets(htmlFile.path, result.outputs);
  }
  
  // Remove separate CSS and JS files since they're now inlined
  const filesToRemove = result.outputs.filter(output => 
    !output.path.endsWith('.html') && 
    (output.path.endsWith('.css') || output.path.endsWith('.js'))
  );
  
  for (const file of filesToRemove) {
    if (existsSync(file.path)) {
      await rm(file.path, { force: true });
    }
  }
  
  console.log("✨ Assets successfully inlined into HTML files");
}

// Print the results
const end = performance.now();

const outputTable = result.outputs
  .filter(output => singleFile ? output.path.endsWith('.html') : true)
  .map(output => ({
    "File": path.relative(process.cwd(), output.path),
    "Type": output.kind,
    "Size": formatFileSize(output.size),
  }));

console.table(outputTable);
const buildTime = (end - start).toFixed(2);

if (singleFile) {
  console.log(`\n✅ Single-file build completed in ${buildTime}ms\n`);
} else {
  console.log(`\n✅ Build completed in ${buildTime}ms\n`);
}
