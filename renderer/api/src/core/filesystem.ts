// Filesystem API for MikoView

import { invokeNative } from './invoke';

export interface FileInfo {
  name: string;
  path: string;
  extension: string;
  size: number;
  modified: number;
  created: number;
  isDirectory: boolean;
  isFile: boolean;
  isSymlink: boolean;
}

export interface DirectoryEntry {
  name: string;
  path: string;
  isDirectory: boolean;
}

export interface ReadFileOptions {
  encoding?: 'utf8' | 'binary' | 'base64';
}

export interface WriteFileOptions {
  encoding?: 'utf8' | 'binary' | 'base64';
  createDirs?: boolean;
}

export interface ReadDirOptions {
  recursive?: boolean;
}

export interface ReadResult {
  success: boolean;
  data: string;
  error?: string;
  encoding: string;
}

export interface WriteResult {
  success: boolean;
  error?: string;
  bytesWritten: number;
}

/**
 * Filesystem operations
 */
export class FileSystem {
  /**
   * Read a file
   */
  static async readFile(path: string, options: ReadFileOptions = {}): Promise<string> {
    const result: ReadResult = await invokeNative('fs.readFile', {
      path,
      encoding: options.encoding || 'utf8'
    });
    
    if (!result.success) {
      throw new Error(result.error || 'Failed to read file');
    }
    
    return result.data;
  }

  /**
   * Write a file
   */
  static async writeFile(path: string, data: string, options: WriteFileOptions = {}): Promise<number> {
    const result: WriteResult = await invokeNative('fs.writeFile', {
      path,
      data,
      encoding: options.encoding || 'utf8',
      createDirs: options.createDirs || false
    });
    
    if (!result.success) {
      throw new Error(result.error || 'Failed to write file');
    }
    
    return result.bytesWritten;
  }

  /**
   * Append to a file
   */
  static async appendFile(path: string, data: string, options: WriteFileOptions = {}): Promise<number> {
    const result: WriteResult = await invokeNative('fs.appendFile', {
      path,
      data,
      encoding: options.encoding || 'utf8',
      createDirs: options.createDirs || false
    });
    
    if (!result.success) {
      throw new Error(result.error || 'Failed to append to file');
    }
    
    return result.bytesWritten;
  }

  /**
   * Delete a file
   */
  static async deleteFile(path: string): Promise<void> {
    const result = await invokeNative('fs.deleteFile', { path });
    
    if (!result.success) {
      throw new Error(result.error || 'Failed to delete file');
    }
  }

  /**
   * Copy a file
   */
  static async copyFile(source: string, destination: string): Promise<void> {
    const result = await invokeNative('fs.copyFile', { source, destination });
    
    if (!result.success) {
      throw new Error(result.error || 'Failed to copy file');
    }
  }

  /**
   * Move/rename a file
   */
  static async moveFile(source: string, destination: string): Promise<void> {
    const result = await invokeNative('fs.moveFile', { source, destination });
    
    if (!result.success) {
      throw new Error(result.error || 'Failed to move file');
    }
  }

  /**
   * Read directory contents
   */
  static async readDir(path: string, options: ReadDirOptions = {}): Promise<DirectoryEntry[]> {
    const entries: DirectoryEntry[] = await invokeNative('fs.readDir', {
      path,
      recursive: options.recursive || false
    });
    
    return entries;
  }

  /**
   * Create a directory
   */
  static async createDir(path: string, recursive: boolean = true): Promise<void> {
    const result = await invokeNative('fs.createDir', { path, recursive });
    
    if (!result.success) {
      throw new Error(result.error || 'Failed to create directory');
    }
  }

  /**
   * Delete a directory
   */
  static async deleteDir(path: string, recursive: boolean = false): Promise<void> {
    const result = await invokeNative('fs.deleteDir', { path, recursive });
    
    if (!result.success) {
      throw new Error(result.error || 'Failed to delete directory');
    }
  }

  /**
   * Get file/directory information
   */
  static async getFileInfo(path: string): Promise<FileInfo> {
    const info: FileInfo = await invokeNative('fs.getFileInfo', { path });
    return info;
  }

  /**
   * Check if a path exists
   */
  static async exists(path: string): Promise<boolean> {
    const result = await invokeNative('fs.exists', { path });
    return result.exists;
  }

  /**
   * Resolve a path
   */
  static async resolvePath(path: string): Promise<string> {
    const result = await invokeNative('fs.resolvePath', { path });
    return result.path;
  }

  /**
   * Get basename of a path
   */
  static async basename(path: string, ext?: string): Promise<string> {
    const result = await invokeNative('fs.basename', { path, ext });
    return result.basename;
  }

  /**
   * Get dirname of a path
   */
  static async dirname(path: string): Promise<string> {
    const result = await invokeNative('fs.dirname', { path });
    return result.dirname;
  }

  /**
   * Get extension of a path
   */
  static async extname(path: string): Promise<string> {
    const result = await invokeNative('fs.extname', { path });
    return result.extname;
  }

  /**
   * Join path segments
   */
  static async joinPath(...segments: string[]): Promise<string> {
    const result = await invokeNative('fs.joinPath', { segments });
    return result.path;
  }
}

// Convenience exports
export const {
  readFile,
  writeFile,
  appendFile,
  deleteFile,
  copyFile,
  moveFile,
  readDir,
  createDir,
  deleteDir,
  getFileInfo,
  exists,
  resolvePath,
  basename,
  dirname,
  extname,
  joinPath
} = FileSystem;

// Default export
export default FileSystem;