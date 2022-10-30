
#pragma once





class DreadSim {
public:
	typedef struct {
		short	A;			// x*A + y*B + C > 0	- right side
		short	B;
		int		C;
		short	left;		// >=0: node index		<0: ~(vis order offset)
		short	right;
	} DataBSPNode;

	typedef struct {
		int		map_collision_bsp_root;
		int		map_hitscan_bsp_root;
	} DataHeader;


	// Engine globals
	short physics_endx;
	short physics_endy;
	short midpoint_endx;
	short midpoint_endy;
	short bspclip_p1x;
	short bspclip_p1y;
	short bspclip_p2x;
	short bspclip_p2y;
	short bspclip_nA;
	short bspclip_nB;
	int   bspclip_nC;
	short bspclip_stuck;

	// Data
	DataHeader _e_map_header;
	DataHeader *e_map_header = &_e_map_header;
	vector<DataBSPNode> e_map_bspnodes;

	// Debug
	bool array_error = false;



	template<class T>
	T *access_array_byte_offset(vector<T> &arr, int byte_offset) {
		static T guard = {};
		if( byte_offset < 0 )
		{
			if( !array_error ) printf("Negative array offset %d\n", byte_offset);
			array_error = true;
			return &guard;
		}
		if( byte_offset % sizeof(T) )
		{
			if( !array_error ) printf("Unaligned array offset %d\n", byte_offset);
			array_error = true;
			return &guard;
		}
		if( dword(byte_offset/sizeof(T)) >= arr.size() )
		{
			if( !array_error ) printf("Array offset %d out of bounds (index %d >= %d elements\n", byte_offset, byte_offset/sizeof(T), arr.size());
			array_error = true;
			return &guard;
		}
		return &arr[byte_offset/sizeof(T)];
	}

	short	BspClipFix(short node_ref);
	short	BspClipFix_Coarse(short node_ref);
	int		BspClipCheckFix(short node_ref);
	short	Physics_WalkSlide(short startx, short starty, short endx, short endy);

	short	New_BspClipFix(short node_ref);
	short	New_Physics_WalkSlide(short startx, short starty, short endx, short endy);
};
