// Core invoke system for MikoView

export interface InvokeRequest {
  method: string;
  data: any;
  requestId?: number;
}

export interface InvokeResponse {
  requestId: number;
  success: boolean;
  data?: any;
  error?: string;
  errorCode?: number;
}

export type InvokeCallback = (result: any, success: boolean) => void;
export type NativeInvokeHandler = (data: any) => Promise<any> | any;

class InvokeManager {
  private static instance: InvokeManager;
  private pendingRequests = new Map<number, InvokeCallback>();
  private nativeHandlers = new Map<string, NativeInvokeHandler>();
  private nextRequestId = 1;

  static getInstance(): InvokeManager {
    if (!InvokeManager.instance) {
      InvokeManager.instance = new InvokeManager();
    }
    return InvokeManager.instance;
  }

  private constructor() {
    // Set up global handlers
    (window as any).mikoview = (window as any).mikoview || {};
    (window as any).mikoview._handleInvokeResponse = this.handleInvokeResponse.bind(this);
    (window as any).mikoview._handleNativeInvoke = this.handleNativeInvoke.bind(this);
  }

  /**
   * Invoke a native method
   */
  async invoke<T = any>(method: string, data?: any): Promise<T> {
    return new Promise<T>((resolve, reject) => {
      const requestId = this.generateRequestId();
      
      this.pendingRequests.set(requestId, (result: any, success: boolean) => {
        if (success) {
          resolve(result);
        } else {
          reject(new Error(result || 'Unknown error'));
        }
      });

      // Call native invoke function
      if ((window as any).mikoview && (window as any).mikoview.invoke) {
        (window as any).mikoview.invoke(method, data || {}, requestId);
      } else {
        // Fallback for development/testing
        setTimeout(() => {
          reject(new Error('Native invoke not available'));
        }, 0);
      }
    });
  }

  /**
   * Register a handler for native-to-renderer invokes
   */
  registerHandler(method: string, handler: NativeInvokeHandler): void {
    this.nativeHandlers.set(method, handler);
  }

  /**
   * Unregister a handler
   */
  unregisterHandler(method: string): void {
    this.nativeHandlers.delete(method);
  }

  private handleInvokeResponse(response: InvokeResponse): void {
    const callback = this.pendingRequests.get(response.requestId);
    if (callback) {
      this.pendingRequests.delete(response.requestId);
      callback(response.success ? response.data : response.error, response.success);
    }
  }

  private async handleNativeInvoke(request: InvokeRequest): Promise<void> {
    const handler = this.nativeHandlers.get(request.method);
    if (!handler) {
      console.warn(`No handler registered for method: ${request.method}`);
      return;
    }

    try {
      const result = await handler(request.data);
      
      // Send response back to native
      if (request.requestId && (window as any).mikoview && (window as any).mikoview.invoke) {
        (window as any).mikoview.invoke('_invokeResponse', {
          requestId: request.requestId,
          success: true,
          data: result
        });
      }
    } catch (error) {
      // Send error response back to native
      if (request.requestId && (window as any).mikoview && (window as any).mikoview.invoke) {
        (window as any).mikoview.invoke('_invokeResponse', {
          requestId: request.requestId,
          success: false,
          error: error instanceof Error ? error.message : String(error)
        });
      }
    }
  }

  private generateRequestId(): number {
    return this.nextRequestId++;
  }
}

// Export the singleton instance
export const invoke = InvokeManager.getInstance();

// Convenience function for direct invocation
export async function invokeNative<T = any>(method: string, data?: any): Promise<T> {
  return invoke.invoke<T>(method, data);
}

// Register handler function
export function registerNativeHandler(method: string, handler: NativeInvokeHandler): void {
  invoke.registerHandler(method, handler);
}

// Unregister handler function
export function unregisterNativeHandler(method: string): void {
  invoke.unregisterHandler(method);
}