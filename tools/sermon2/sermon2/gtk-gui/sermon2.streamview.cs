// ------------------------------------------------------------------------------
//  <autogenerated>
//      This code was generated by a tool.
//      Mono Runtime Version: 2.0.50727.42
// 
//      Changes to this file may cause incorrect behavior and will be lost if 
//      the code is regenerated.
//  </autogenerated>
// ------------------------------------------------------------------------------

namespace sermon2 {
    
    
    public partial class streamview {
        
        private Gtk.Action FileAction;
        
        private Gtk.Action EditAction;
        
        private Gtk.Action ClearAction;
        
        private Gtk.Action loremIpsumAction;
        
        private Gtk.VBox vbox1;
        
        private Gtk.MenuBar menubar1;
        
        private Gtk.Frame frame1;
        
        private Gtk.Alignment GtkAlignment;
        
        private Gtk.ScrolledWindow GtkScrolledWindow1;
        
        private sermon2.hexview hexview1;
        
        private Gtk.Label GtkLabel3;
        
        private Gtk.Statusbar statusbar1;
        
        protected virtual void Build() {
            Stetic.Gui.Initialize(this);
            // Widget sermon2.streamview
            Gtk.UIManager w1 = new Gtk.UIManager();
            Gtk.ActionGroup w2 = new Gtk.ActionGroup("Default");
            this.FileAction = new Gtk.Action("FileAction", Mono.Unix.Catalog.GetString("File"), null, null);
            this.FileAction.ShortLabel = Mono.Unix.Catalog.GetString("File");
            w2.Add(this.FileAction, null);
            this.EditAction = new Gtk.Action("EditAction", Mono.Unix.Catalog.GetString("Edit"), null, null);
            this.EditAction.ShortLabel = Mono.Unix.Catalog.GetString("Edit");
            w2.Add(this.EditAction, null);
            this.ClearAction = new Gtk.Action("ClearAction", Mono.Unix.Catalog.GetString("Clear"), null, "gtk-cancel");
            this.ClearAction.ShortLabel = Mono.Unix.Catalog.GetString("Clear");
            w2.Add(this.ClearAction, null);
            this.loremIpsumAction = new Gtk.Action("loremIpsumAction", Mono.Unix.Catalog.GetString("lorem ipsum"), null, "gtk-spell-check");
            this.loremIpsumAction.ShortLabel = Mono.Unix.Catalog.GetString("lorem ipsum");
            w2.Add(this.loremIpsumAction, null);
            w1.InsertActionGroup(w2, 0);
            this.AddAccelGroup(w1.AccelGroup);
            this.Name = "sermon2.streamview";
            this.Title = Mono.Unix.Catalog.GetString("streamview");
            this.WindowPosition = ((Gtk.WindowPosition)(4));
            this.BorderWidth = ((uint)(3));
            // Container child sermon2.streamview.Gtk.Container+ContainerChild
            this.vbox1 = new Gtk.VBox();
            this.vbox1.Name = "vbox1";
            this.vbox1.Spacing = 6;
            // Container child vbox1.Gtk.Box+BoxChild
            w1.AddUiFromString("<ui><menubar name='menubar1'><menu action='FileAction'/><menu action='EditAction'><menuitem action='ClearAction'/><menuitem action='loremIpsumAction'/></menu></menubar></ui>");
            this.menubar1 = ((Gtk.MenuBar)(w1.GetWidget("/menubar1")));
            this.menubar1.Name = "menubar1";
            this.vbox1.Add(this.menubar1);
            Gtk.Box.BoxChild w3 = ((Gtk.Box.BoxChild)(this.vbox1[this.menubar1]));
            w3.Position = 0;
            w3.Expand = false;
            w3.Fill = false;
            // Container child vbox1.Gtk.Box+BoxChild
            this.frame1 = new Gtk.Frame();
            this.frame1.Name = "frame1";
            this.frame1.ShadowType = ((Gtk.ShadowType)(1));
            // Container child frame1.Gtk.Container+ContainerChild
            this.GtkAlignment = new Gtk.Alignment(0F, 0F, 1F, 1F);
            this.GtkAlignment.Name = "GtkAlignment";
            this.GtkAlignment.LeftPadding = ((uint)(12));
            // Container child GtkAlignment.Gtk.Container+ContainerChild
            this.GtkScrolledWindow1 = new Gtk.ScrolledWindow();
            this.GtkScrolledWindow1.Name = "GtkScrolledWindow1";
            this.GtkScrolledWindow1.VscrollbarPolicy = ((Gtk.PolicyType)(2));
            this.GtkScrolledWindow1.HscrollbarPolicy = ((Gtk.PolicyType)(2));
            this.GtkScrolledWindow1.ShadowType = ((Gtk.ShadowType)(1));
            // Container child GtkScrolledWindow1.Gtk.Container+ContainerChild
            Gtk.Viewport w4 = new Gtk.Viewport();
            w4.ShadowType = ((Gtk.ShadowType)(0));
            // Container child GtkViewport1.Gtk.Container+ContainerChild
            this.hexview1 = new sermon2.hexview();
            this.hexview1.WidthRequest = 330;
            this.hexview1.HeightRequest = 400;
            this.hexview1.Events = ((Gdk.EventMask)(256));
            this.hexview1.ExtensionEvents = ((Gdk.ExtensionMode)(1));
            this.hexview1.Name = "hexview1";
            w4.Add(this.hexview1);
            this.GtkScrolledWindow1.Add(w4);
            this.GtkAlignment.Add(this.GtkScrolledWindow1);
            this.frame1.Add(this.GtkAlignment);
            this.GtkLabel3 = new Gtk.Label();
            this.GtkLabel3.Name = "GtkLabel3";
            this.GtkLabel3.LabelProp = Mono.Unix.Catalog.GetString("<b>stream</b>");
            this.GtkLabel3.UseMarkup = true;
            this.frame1.LabelWidget = this.GtkLabel3;
            this.vbox1.Add(this.frame1);
            Gtk.Box.BoxChild w9 = ((Gtk.Box.BoxChild)(this.vbox1[this.frame1]));
            w9.Position = 1;
            // Container child vbox1.Gtk.Box+BoxChild
            this.statusbar1 = new Gtk.Statusbar();
            this.statusbar1.Name = "statusbar1";
            this.statusbar1.Spacing = 6;
            this.vbox1.Add(this.statusbar1);
            Gtk.Box.BoxChild w10 = ((Gtk.Box.BoxChild)(this.vbox1[this.statusbar1]));
            w10.Position = 2;
            w10.Expand = false;
            w10.Fill = false;
            this.Add(this.vbox1);
            if ((this.Child != null)) {
                this.Child.ShowAll();
            }
            this.DefaultWidth = 400;
            this.DefaultHeight = 452;
            this.Show();
            this.ClearAction.Activated += new System.EventHandler(this.OnClearActionActivated);
            this.loremIpsumAction.Activated += new System.EventHandler(this.OnLoremIpsumActionActivated);
        }
    }
}
