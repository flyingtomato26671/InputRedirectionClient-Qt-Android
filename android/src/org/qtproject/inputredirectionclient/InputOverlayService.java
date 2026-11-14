package org.qtproject.inputredirectionclient;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.IBinder;
import android.provider.Settings;
import android.util.Log;
import android.view.Gravity;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

import org.qtproject.qt5.android.QtNative;

/**
 * Servicio en primer plano (mantiene vivo el proceso y el envio UDP a la 3DS)
 * con una ventana flotante de 1 px que tiene el foco de entrada. Android solo
 * entrega el gamepad a la ventana enfocada; esta ventana reenvia cada evento
 * a Qt, donde QtGamepad lo procesa igual que en primer plano.
 */
public class InputOverlayService extends Service {
    private static final String TAG = "IRC-Overlay";
    private static final String CHANNEL_ID = "irc_background";
    private static final int NOTIFICATION_ID = 1;

    private View overlay;

    /** Arranca el servicio si tenemos permiso de superposicion. */
    public static void show(Context c) {
        if (Build.VERSION.SDK_INT >= 23 && !Settings.canDrawOverlays(c))
            return;
        Intent i = new Intent(c, InputOverlayService.class);
        try {
            if (Build.VERSION.SDK_INT >= 26)
                Api26.startForegroundService(c, i);
            else
                c.startService(i);
        } catch (RuntimeException e) {
            Log.w(TAG, "No se pudo iniciar el servicio", e);
        }
    }

    public static void hide(Context c) {
        try {
            c.stopService(new Intent(c, InputOverlayService.class));
        } catch (RuntimeException e) {
            Log.w(TAG, "No se pudo detener el servicio", e);
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        // Debe llamarse enseguida tras startForegroundService().
        startForeground(NOTIFICATION_ID, buildNotification());
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        addOverlay();
        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        removeOverlay();
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private Notification buildNotification() {
        Notification.Builder b;
        if (Build.VERSION.SDK_INT >= 26)
            b = Api26.newNotificationBuilder(this, CHANNEL_ID, "Gamepad in background");
        else
            b = new Notification.Builder(this);

        b.setContentTitle("InputRedirectionClient")
                .setContentText("Sending gamepad input to the 3DS")
                .setSmallIcon(getApplicationInfo().icon)
                .setOngoing(true);

        Intent launch = getPackageManager().getLaunchIntentForPackage(getPackageName());
        if (launch != null) {
            int piFlags = PendingIntent.FLAG_UPDATE_CURRENT;
            if (Build.VERSION.SDK_INT >= 23)
                piFlags |= PendingIntent.FLAG_IMMUTABLE;
            b.setContentIntent(PendingIntent.getActivity(this, 0, launch, piFlags));
        }
        return b.build();
    }

    private void addOverlay() {
        if (overlay != null)
            return;
        try {
            WindowManager wm = (WindowManager) getSystemService(WINDOW_SERVICE);
            int type = Build.VERSION.SDK_INT >= 26
                    ? Api26.overlayWindowType()
                    : WindowManager.LayoutParams.TYPE_PHONE;
            // Sin FLAG_NOT_FOCUSABLE: la ventana tiene el foco y recibe el gamepad.
            // FLAG_NOT_TOUCHABLE: nunca intercepta toques. FLAG_ALT_FOCUSABLE_IM:
            // no interfiere con el teclado en pantalla de la app que esta debajo.
            int flags = WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                    | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                    | WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM;
            WindowManager.LayoutParams lp =
                    new WindowManager.LayoutParams(1, 1, type, flags, PixelFormat.TRANSLUCENT);
            lp.gravity = Gravity.TOP | Gravity.START;
            overlay = new GamepadSinkView(this);
            wm.addView(overlay, lp);
            overlay.requestFocus();
        } catch (RuntimeException e) {
            Log.w(TAG, "No se pudo crear la ventana flotante", e);
            overlay = null;
        }
    }

    private void removeOverlay() {
        if (overlay == null)
            return;
        try {
            ((WindowManager) getSystemService(WINDOW_SERVICE)).removeView(overlay);
        } catch (RuntimeException e) {
            Log.w(TAG, "No se pudo quitar la ventana flotante", e);
        }
        overlay = null;
    }

    /** Vista invisible que reenvia los eventos de gamepad a Qt. */
    private static final class GamepadSinkView extends View {
        private final JoystickDpad joystickDpad = new JoystickDpad();

        GamepadSinkView(Context c) {
            super(c);
            setFocusable(true);
            setFocusableInTouchMode(true);
        }

        private static boolean isGamepad(int source) {
            return (source & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD
                    || (source & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK
                    || (source & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD;
        }

        @Override
        public boolean dispatchKeyEvent(KeyEvent event) {
            if (isGamepad(event.getSource())) {
                try {
                    return QtNative.dispatchKeyEvent(event);
                } catch (Throwable t) { // Qt aun no esta cargado
                    return false;
                }
            }
            return super.dispatchKeyEvent(event);
        }

        @Override
        public boolean dispatchGenericMotionEvent(MotionEvent event) {
            if (JoystickDpad.isJoystickMove(event)) {
                try {
                    QtNative.dispatchGenericMotionEvent(event);
                } catch (Throwable t) { // Qt aun no esta cargado
                    return false;
                }
                // Consumido siempre: asi Android no convierte el stick en flechas.
                joystickDpad.syncHat(event, new JoystickDpad.KeySink() {
                    @Override
                    public void send(KeyEvent e) {
                        try {
                            QtNative.dispatchKeyEvent(e);
                        } catch (Throwable t) {
                            // ignorado
                        }
                    }
                });
                return true;
            }
            return super.dispatchGenericMotionEvent(event);
        }
    }
}
