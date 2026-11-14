package org.qtproject.inputredirectionclient;

import android.view.KeyEvent;

import java.util.HashMap;
import java.util.Map;

/**
 * Convierte los ejes de la cruceta (AXIS_HAT_X / AXIS_HAT_Y) en pulsaciones de
 * flechas DPAD, una por cada cambio de direccion. Logica pura, sin objetos de
 * Android en tiempo de ejecucion, para poder probarla en una JVM normal.
 */
final class HatDpad {
    interface Out {
        void key(int action, int keyCode, int deviceId);
    }

    private static final float THRESHOLD = 0.5f;

    // Ultima direccion (-1, 0, 1) por dispositivo: [x, y]
    private final Map<Integer, int[]> state = new HashMap<Integer, int[]>();

    static int direction(float axisValue) {
        if (axisValue > THRESHOLD)
            return 1;
        if (axisValue < -THRESHOLD)
            return -1;
        return 0;
    }

    void update(int deviceId, float hatX, float hatY, Out out) {
        int[] prev = state.get(deviceId);
        if (prev == null) {
            prev = new int[] {0, 0};
            state.put(deviceId, prev);
        }
        change(prev, 0, direction(hatX), KeyEvent.KEYCODE_DPAD_LEFT, KeyEvent.KEYCODE_DPAD_RIGHT, deviceId, out);
        change(prev, 1, direction(hatY), KeyEvent.KEYCODE_DPAD_UP, KeyEvent.KEYCODE_DPAD_DOWN, deviceId, out);
    }

    private static void change(int[] prev, int axis, int now, int negKey, int posKey, int deviceId, Out out) {
        int before = prev[axis];
        if (now == before)
            return;
        prev[axis] = now;
        if (before != 0)
            out.key(KeyEvent.ACTION_UP, before < 0 ? negKey : posKey, deviceId);
        if (now != 0)
            out.key(KeyEvent.ACTION_DOWN, now < 0 ? negKey : posKey, deviceId);
    }
}
