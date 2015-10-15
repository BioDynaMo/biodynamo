package ini.cx3d.utilities;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.AbstractSequentialList;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Functions used for convenient debugging of ported classes
 */
public class DebugUtil {
    public static boolean print = true;
    protected static AtomicInteger printedLines = new AtomicInteger(0);

    public static Object observed = null;

    /**
     * This method creates a dynamic java proxy who intercepts all method calls specified in the param interfaces
     *
     * @param delegate   object instance that should be procied
     * @param interfaces specifies methods that should be intercepted
     * @param <T>        Type of the delegate
     * @return proxy implementing the given interfaces
     */
    public static <T> T createDebugLoggingProxy(final T delegate, Class[] interfaces) {
        if (delegate == null) {
            return null;
        }
        DebugLoggingHandler handler = new DebugLoggingHandler(delegate);
        T ret = (T) Proxy.newProxyInstance(
                interfaces[0].getClassLoader(),
                interfaces,
                handler);
        handler.setProxy(ret);
        return ret;
    }

    /**
     * Handler that is called instead of the proxied methods
     * It logs method calls, their arguments, the objects inner state as well as the return type
     */
    static class DebugLoggingHandler implements InvocationHandler {

        private Object delegate = null;
        // this variable holds the reference to the proxy object and is
        // needed to filter output for a certain object
        // e.g. if(Integer.toHexString(System.identityHashCode(proxy)).equals("3bd94634"))
        private Object proxy = null;


        public DebugLoggingHandler(Object proxy) {
            this.delegate = proxy;
        }

        @Override
        public Object invoke(Object o, Method method, Object[] objects) throws Throwable {
            if(DebugUtil.print && !method.getName().equals("toString")){
                logMethodCall(method, objects);
            }

            Object result = null;
            if (method.getName().equals("equals") && objects[0] instanceof Proxy) {
                // fixes issue if proxy.equals(otherProxy)
                // inside the equals implementation of the proxied class the conditional: if(other instanceof Proxied)
                // will fail, because it it of type Proxy/ProxiedInterface but not Proxied
                // therefore we prevent this case here.
                result = objects[0].equals(delegate);
            } else {
                result = method.invoke(delegate, objects);
                if(Integer.toHexString(System.identityHashCode(proxy)).equals("46238e3f")){
                    DebugUtil.observed = proxy;
                }
            }

            if(DebugUtil.print && !method.getName().equals("toString")){
                logMethodReturn(method, result);
            }
            return result;
        }

        public void setProxy(Object proxy) {
            this.proxy = proxy;
        }

        private void logMethodCall(Method method, Object[] objects) {
            int line = printedLines.incrementAndGet();
            String methodCallLogMsg = "DBG L#" +line+ " " + method.getName() + " args: " + processArguments(objects) + " innerState: " + proxy;
            System.out.println(methodCallLogMsg);
        }

        private void logMethodReturn(Method method, Object result) {
            StringBuilder sb = new StringBuilder();
            processArgument(sb, result);
            int line = printedLines.incrementAndGet();
            String methodReturnLogMsg = "DBG L#"+line+" "+method.getName()+" return " + sb.toString() + " innerState: " + proxy;
            System.out.println(methodReturnLogMsg);
        }

        private String processArguments(Object[] args) {
            if (args == null) {
                return "";
            }
            StringBuilder sb = new StringBuilder();
            sb.append("{");
            for (Object arg : args) {
                if (arg != null) {
                    processArgument(sb, arg);
                } else {
                    sb.append("null");
                }
                sb.append(", ");
            }
            sb.append("}");
            return sb.toString();
        }

        private void processArgument(StringBuilder sb, Object arg) {
            if (arg == null) {
                return;
            } else if (arg instanceof Double) {
                sb.append(StringUtilities.toStr((double) arg));
            } else if (arg instanceof double[]) {
                sb.append(StringUtilities.toStr((double[]) arg));
            } else if (arg instanceof double[][]) {
                sb.append(StringUtilities.toStr((double[][]) arg));
            } else if (arg instanceof Object[]) {
                sb.append(StringUtilities.toStr((Object[]) arg));
            } else if (arg instanceof AbstractSequentialList) {
                sb.append(StringUtilities.toStr((AbstractSequentialList) arg));
            } else {
                sb.append(arg);
            }
        }
    }
}
