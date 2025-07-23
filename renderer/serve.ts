import { config } from './config'
import { spawn } from 'child_process'

function run(command: string, cwd: string) {
  const [cmd, ...args] = command.split(' ')
  const proc = spawn(cmd, args, { cwd, stdio: 'inherit', shell: true })
  proc.on('exit', code => {
    process.exit(code ?? 0)
  })
}

if (config.framework === 'react') {
  console.log('🌀 Launching React Dev Server...')
  run('bun run dev', './react')
} else if (config.framework === 'vue') {
  console.log('🍵 Launching Vue Dev Server...')
  run('bun run dev', './vue')
}
