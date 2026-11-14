package org.qtproject.inputredirectionclient;

import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.Toast;

import org.qtproject.qt5.android.bindings.QtActivity;

/**
 * Actividad de Qt con soporte para recibir el gamepad en segundo plano:
 * al pasar a segundo plano arranca un servicio con una ventana flotante
 * invisible que recibe el gamepad y se lo entrega a Qt.
 */
public class MainActivity extends QtActivity {
    private static final String PREFS = "irc_prefs";
    private static final String KEY_ASKED = "asked_overlay_permission";

    private final JoystickDpad joystickDpad = new JoystickDpad();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        askOverlayPermissionOnce();
    }

    @Override
    protected void onResume() {
        super.onResume();
        InputOverlayService.hide(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
        InputOverlayService.show(this);
    }

    /**
     * Entrega el joystick a Qt y lo da por atendido para que Android no lo
     * convierta en flechas DPAD (el stick izquierdo no debe actuar como cruceta).
     * La cruceta (hat) si genera flechas, via {@link JoystickDpad}.
     */
    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent ev) {
        boolean handled = super.dispatchGenericMotionEvent(ev);
        if (JoystickDpad.isJoystickMove(ev)) {
            joystickDpad.syncHat(ev, new JoystickDpad.KeySink() {
                @Override
                public void send(KeyEvent event) {
                    dispatchKeyEvent(event);
                }
            });
            return true;
        }
        return handled;
    }

    /** Pide una sola vez el permiso "Mostrar sobre otras apps". */
    private void askOverlayPermissionOnce() {
        if (Build.VERSION.SDK_INT < 23 || Settings.canDrawOverlays(this))
            return;
        SharedPreferences prefs = getSharedPreferences(PREFS, MODE_PRIVATE);
        if (prefs.getBoolean(KEY_ASKED, false))
            return;
        prefs.edit().putBoolean(KEY_ASKED, true).apply();
        Toast.makeText(this,
                "Enable \"Display over other apps\" to keep using the gamepad in the background",
                Toast.LENGTH_LONG).show();
        try {
            startActivity(new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
                    Uri.parse("package:" + getPackageName())));
        } catch (RuntimeException ignored) {
        }
    }
}
