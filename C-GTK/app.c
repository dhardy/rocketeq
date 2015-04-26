#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include <gtk/gtk.h>

#ifndef NAN
#define NAN (0.0 / 0.0)
#endif

size_t LEN = 16;        // buffer length
enum {
    UNIT_MS = 0,    // meters per second
    UNIT_KPH = 1,   // kilometers per hour
    UNIT_MPH = 2    // miles per hour
};

double G0 = 9.81;       // m / s^2
double KPH_PER_MS = 3.6;        // 1 m/s = 3.6 kph
double MPH_PER_MS = 2.23694;    // 1 m/s in mph

GtkWidget *en_wm = NULL,
    *en_dm = NULL,
    *en_isp = NULL,
    *en_dv = NULL,
    *ct_dv = NULL;

static double try_readd(const char *str) {
    char *end;
    double v = strtod(str, &end);
    if ((end - str == 0) || end - str < strlen(str)) return NAN;
    else return v;
}

static void calculate(GtkWidget *entry, gpointer data) {
    assert (en_wm != NULL && en_dm != NULL &&
        en_isp != NULL && en_dv != NULL &&
        ct_dv != NULL && "Widget not initialised!");
    
    double wm = try_readd(gtk_entry_get_text (GTK_ENTRY(en_wm)));
    double dm = try_readd(gtk_entry_get_text (GTK_ENTRY(en_dm)));
    
    double isp = try_readd(gtk_entry_get_text (GTK_ENTRY(en_isp)));
    double ve = isp * G0;
    
    double dv = try_readd(gtk_entry_get_text (GTK_ENTRY(en_dv)));
    gint unit = gtk_combo_box_get_active (GTK_COMBO_BOX(ct_dv));
    // back-convert from whatever unit to m/s:
    if (unit == UNIT_KPH) dv /= KPH_PER_MS;
    else if (unit == UNIT_MPH) dv /= MPH_PER_MS;
    else if (unit != UNIT_MS) assert (0 && "Unexpected unit!");
    
    gchar buf[LEN];
    
    if (!(wm > 0.0) && entry != en_wm) {
        if (dm != dm || ve != ve || dv != dv) return;   // NaNs; stop
        wm = dm * exp(dv / ve);
        g_snprintf (buf, LEN, "%f", wm);
        gtk_entry_set_text (GTK_ENTRY(en_wm), buf);
    } else if (!(dm > 0.0) && entry != en_dm) {
        if (wm != wm || ve != ve || dv != dv) return;   // NaNs; stop
        dm = wm / exp(dv / ve);
        g_snprintf (buf, LEN, "%f", dm);
        gtk_entry_set_text (GTK_ENTRY(en_dm), buf);
    } else if (!(isp > 0.0) && entry != en_isp) {
        if (dm != dm || wm != wm || dv != dv) return;   // NaNs; stop
        isp = dv / (log(wm / dm) * G0);
        g_snprintf (buf, LEN, "%f", isp);
        gtk_entry_set_text (GTK_ENTRY(en_isp), buf);
    } else if (entry != en_dv) {
        if (dm != dm || wm != wm || ve != ve) return;   // NaNs; stop
        dv = ve * log(wm / dm);
        if (unit == UNIT_KPH) dv *= KPH_PER_MS;
        else if (unit == UNIT_MPH) dv *= MPH_PER_MS;
        
        gchar *unit = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(ct_dv));
        g_snprintf (buf, LEN, "%f", dv);
        gtk_entry_set_text (GTK_ENTRY(en_dv), buf);
    } // else do nothing
}

static void activate (GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Rocket Equation (C-GTK)");
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 200);;
    
    GtkWidget *gd_top = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID(gd_top), 8);
    gtk_container_add (GTK_CONTAINER (window), gd_top);
    
    GtkWidget *lb_tit = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL(lb_tit), "<span size=\"large\">Rocket Equation calculator</span>");
    gtk_grid_attach (GTK_GRID(gd_top), lb_tit, 0, 0 /*col 0 row 0*/, 3, 1 /*col, row span*/);
    
    GtkWidget *lb_eq = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL(lb_eq),
        "Δv = g<sub>0</sub> I<sub>sp</sub> ln(m<sub>0</sub> / m<sub>1</sub>)");
    gtk_grid_attach (GTK_GRID(gd_top), lb_eq, 0, 1 /*col 0 row 0*/, 3, 1 /*col, row span*/);
    
    // "wet mass" line
    GtkWidget *lb_wm = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL(lb_wm), "Wet mass (m<sub>0</sub>):");
    gtk_grid_attach (GTK_GRID(gd_top), lb_wm, 0, 2, 1, 1);
    en_wm = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY(en_wm), "150");
    gtk_grid_attach (GTK_GRID(gd_top), en_wm, 1, 2, 1, 1);
    
    // "dry mass" line
    GtkWidget *lb_dm = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL(lb_dm), "Dry mass (m<sub>1</sub>):");
    gtk_grid_attach (GTK_GRID(gd_top), lb_dm, 0, 3, 1, 1);
    en_dm = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY(en_dm), "100");
    gtk_grid_attach (GTK_GRID(gd_top), en_dm, 1, 3, 1, 1);
    
    // dry/wet mass unit
    GtkWidget *lb_mu = gtk_label_new ("Any unit of mass (both must use the same)");
    gtk_label_set_line_wrap (GTK_LABEL(lb_mu), TRUE);
    gtk_grid_attach (GTK_GRID(gd_top), lb_mu, 2, 2, 1, 2);
    
    // engine Isp line
    GtkWidget *lb_isp = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL(lb_isp), "Engine I<sub>sp</sub>:");
    gtk_grid_attach (GTK_GRID(gd_top), lb_isp, 0, 4, 1, 1);
    en_isp = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY(en_isp), "100");
    gtk_grid_attach (GTK_GRID(gd_top), en_isp, 1, 4, 1, 1);
    GtkWidget *lb_u_isp = gtk_label_new ("s");
    gtk_grid_attach (GTK_GRID(gd_top), lb_u_isp, 2, 4, 1, 1);
    
    // Δv line
    GtkWidget *lb_dv = gtk_label_new ("Δv:");
    gtk_grid_attach (GTK_GRID(gd_top), lb_dv, 0, 5, 1, 1);
    en_dv = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID(gd_top), en_dv, 1, 5, 1, 1);
    ct_dv = gtk_combo_box_text_new ();
    // orders must match the UNIT_xyz enum items declared above
    gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(ct_dv), NULL, "m/s");
    gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(ct_dv), NULL, "km/h");
    gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(ct_dv), NULL, "mph");
    gtk_combo_box_set_active (GTK_COMBO_BOX(ct_dv), 0);
    gtk_grid_attach (GTK_GRID(gd_top), ct_dv, 2, 5, 1, 1);
    
    GtkWidget *lb_usage = gtk_label_new ("If any box is empty, that is calculated; otherwise Δv is calculated.");
    gtk_grid_attach (GTK_GRID(gd_top), lb_usage, 0, 6, 3, 1);
    
    g_signal_connect_after (en_wm, "changed", G_CALLBACK (calculate), NULL);
    g_signal_connect_after (en_dm, "changed", G_CALLBACK (calculate), NULL);
    g_signal_connect_after (en_isp, "changed", G_CALLBACK (calculate), NULL);
    g_signal_connect_after (en_dv, "changed", G_CALLBACK (calculate), NULL);
    g_signal_connect_after (ct_dv, "changed", G_CALLBACK (calculate), NULL);
    
    calculate (en_dv, NULL);
    
    gtk_widget_show_all (window);
}

int main (int argc, char **argv) {
    GtkApplication *app = gtk_application_new ("rocketeq.c-qtk", G_APPLICATION_FLAGS_NONE);
    int status = 0;
    
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    
    return status;
}
