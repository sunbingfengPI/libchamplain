/* champlain-memphis-0.6.vapi generated by vapigen, do not modify. */

[CCode (cprefix = "Champlain", lower_case_cprefix = "champlain_")]
namespace Champlain {
	[Compact]
	[CCode (copy_function = "champlain_bounding_box_copy", type_id = "CHAMPLAIN_TYPE_BOUNDING_BOX", cheader_filename = "champlain/champlain.h")]
	public class BoundingBox {
		public double bottom;
		public double left;
		public double right;
		public double top;
		[CCode (has_construct_function = false)]
		public BoundingBox ();
		public unowned Champlain.BoundingBox copy ();
		public void get_center (out double lat, out double lon);
	}
	[CCode (cheader_filename = "champlain/champlain.h")]
	public class LocalMapDataSource : Champlain.MapDataSource {
		[CCode (has_construct_function = false)]
		public LocalMapDataSource ();
		public void load_map_data (string map_path);
	}
	[CCode (cheader_filename = "champlain/champlain.h")]
	public class MapDataSource : GLib.InitiallyUnowned {
		public virtual unowned Memphis.Map get_map_data ();
		[NoAccessorMethod]
		public Champlain.BoundingBox bounding_box { owned get; set; }
		[NoAccessorMethod]
		public Champlain.State state { get; set; }
	}
	[CCode (cheader_filename = "champlain/champlain.h")]
	public class MemphisTileSource : Champlain.TileSource {
		[CCode (has_construct_function = false)]
		public MemphisTileSource.full (string id, string name, string license, string license_uri, uint min_zoom, uint max_zoom, uint tile_size, Champlain.MapProjection projection, Champlain.MapDataSource map_data_source);
		public Clutter.Color get_background_color ();
		public unowned Champlain.MapDataSource get_map_data_source ();
		public unowned Memphis.Rule get_rule (string id);
		public unowned GLib.List get_rule_ids ();
		public void load_rules (string rules_path);
		public void remove_rule (string id);
		public void set_background_color (Clutter.Color color);
		public void set_map_data_source (Champlain.MapDataSource map_data_source);
		public void set_rule (Memphis.Rule rule);
		public Champlain.MapDataSource map_data_source { get; set construct; }
	}
	[CCode (cheader_filename = "champlain/champlain.h")]
	public class NetworkMapDataSource : Champlain.MapDataSource {
		[CCode (has_construct_function = false)]
		public NetworkMapDataSource ();
		public unowned string get_api_uri ();
		public void load_map_data (double bound_left, double bound_bottom, double bound_right, double bound_top);
		public void set_api_uri (string api_uri);
		public string api_uri { get; set; }
		[NoAccessorMethod]
		public string proxy_uri { owned get; set; }
	}
}
