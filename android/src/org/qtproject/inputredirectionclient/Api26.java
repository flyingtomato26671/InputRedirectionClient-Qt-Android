package org.qtproject.inputredirectionclient;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.view.WindowManager;

/** Llamadas que solo existen desde Android 8 (API 26); se usan tras comprobar la version. */
final class Api26 {
    private Api26() {}

    static void startForegroundService(Context c, Intent i) {
        c.startForegroundService(i);
    }

    static Notification.Builder newNotificationBuilder(Context c, String channelId, String channelName) {
        NotificationManager nm = (NotificationManager) c.getSystemService(Context.NOTIFICATION_SERVICE);
        nm.createNotificationChannel(
                new NotificationChannel(channelId, channelName, NotificationManager.IMPORTANCE_LOW));
        return new Notification.Builder(c, channelId);
    }

    static int overlayWindowType() {
        return WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
    }
}
