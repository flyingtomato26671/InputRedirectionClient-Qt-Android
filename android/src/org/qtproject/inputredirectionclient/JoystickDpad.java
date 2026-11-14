package org.qtproject.inputredirectionclient;

import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;

/**
 * Qt (QtGamepad) no marca como atendidos los eventos del joystick. Entonces
 * Android los convierte solo en flechas DPAD: el stick izquierdo acaba
 * apareciendo como cruceta. Aqui esos eventos se consumen (se entregan a Qt y
 * se devuelve true) y solo la cruceta real (hat) genera flechas.
 */
final class JoystickDpad {
    interface KeySink {
        void send(KeyEvent event);
    }

    private final HatDpad hat = new HatDpad();

    /** Evento de movimiento de un joystick/gamepad (sticks, gatillos y cruceta). */
    static boolean isJoystickMove(MotionEvent ev) {
        return (ev.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0
                && ev.getAction() == MotionEvent.ACTION_MOVE;
    }

    /** Genera flechas DPAD a partir de la cruceta (hat) del evento. */
    void syncHat(final MotionEvent ev, final KeySink sink) {
        final long time = ev.getEventTime();
        final int source = ev.getSource();
        hat.update(ev.getDeviceId(),
                ev.getAxisValue(MotionEvent.AXIS_HAT_X),
                ev.getAxisValue(MotionEvent.AXIS_HAT_Y),
                new HatDpad.Out() {
                    @Override
                    public void key(int action, int keyCode, int deviceId) {
                        sink.send(new KeyEvent(time, time, action, keyCode, 0, 0, deviceId, 0, 0, source));
                    }
                });
    }
}
