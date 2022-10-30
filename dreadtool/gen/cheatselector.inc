	if(  line->tex_ceil && line->h1== -64 && line->h2==   0 ) fm=  0;	//	Ceil  64   0  front
	if(  line->tex_ceil && line->h1== -64 && line->h2==   0 ) cm=  1;	//	Ceil  64   0  CLIPPED
	if( !line->tex_ceil && line->h1== -64 && line->h2==   0 ) fm=  2;	//	Sky   64   0  front
	if( !line->tex_ceil && line->h1== -64 && line->h2==   0 ) cm=  3;	//	Sky   64   0  CLIPPED
	if(  line->tex_ceil && line->h1==-128 && line->h2==   0 ) fm=  4;	//	Ceil 128   0  front
	if(  line->tex_ceil && line->h1==-128 && line->h2==   0 ) cm=  5;	//	Ceil 128   0  CLIPPED
	if( !line->tex_ceil && line->h1==-128 && line->h2==   0 ) fm=  6;	//	Sky  128   0  front
	if( !line->tex_ceil && line->h1==-128 && line->h2==   0 ) cm=  7;	//	Sky  128   0  CLIPPED
	if(  line->tex_ceil && line->h1==-128 && line->h2== -64 ) fm=  8;	//	Ceil 128  64  front
	if(  line->tex_ceil && line->h1==-128 && line->h2== -64 ) cm=  9;	//	Ceil 128  64  CLIPPED
	if( !line->tex_ceil && line->h1==-128 && line->h2== -64 ) fm= 10;	//	Sky  128  64  front
	if( !line->tex_ceil && line->h1==-128 && line->h2== -64 ) cm= 11;	//	Sky  128  64  CLIPPED
	if(  line->tex_ceil && line->h1== -64 && line->h2== -64 ) fm= 12;	//	Ceil  64  64  front
	if(  line->tex_ceil && line->h1== -64 && line->h2== -64 ) cm= 13;	//	Ceil  64  64  CLIPPED
	if(  line->tex_ceil && line->h1== -96 && line->h2==   0 ) fm= 14;	//	Ceil  96   0  front
	if(  line->tex_ceil && line->h1== -96 && line->h2==   0 ) cm= 15;	//	Ceil  96   0  CLIPPED
	if( !line->tex_ceil && line->h1== -96 && line->h2==   0 ) fm= 16;	//	Sky   96   0  front
	if( !line->tex_ceil && line->h1== -96 && line->h2==   0 ) cm= 17;	//	Sky   96   0  CLIPPED
	if(  line->tex_ceil && line->h1== -96 && line->h2== -64 ) fm= 18;	//	Ceil  96  64  front
	if(  line->tex_ceil && line->h1== -96 && line->h2== -64 ) cm= 19;	//	Ceil  96  64  CLIPPED
	if( !line->tex_ceil && line->h1== -96 && line->h2== -64 ) fm= 20;	//	Sky   96  64  front
	if( !line->tex_ceil && line->h1== -96 && line->h2== -64 ) cm= 21;	//	Sky   96  64  CLIPPED
	if(  line->tex_ceil && line->h1==-128 && line->h2== -96 ) fm= 22;	//	Ceil 128  96  front
	if(  line->tex_ceil && line->h1==-128 && line->h2== -96 ) cm= 23;	//	Ceil 128  96  CLIPPED
	if( !line->tex_ceil && line->h1==-128 && line->h2== -96 ) fm= 24;	//	Sky  128  96  front
	if( !line->tex_ceil && line->h1==-128 && line->h2== -96 ) cm= 25;	//	Sky  128  96  CLIPPED
	if(  line->tex_ceil && line->h1== -96 && line->h2== -96 ) fm= 26;	//	Ceil  96  96  front
	if(  line->tex_ceil && line->h1== -96 && line->h2== -96 ) cm= 27;	//	Ceil  96  96  CLIPPED
	if(  line->tex_ceil && line->h1==-128 && line->h2==-128 ) fm= 28;	//	Ceil 128 128  front
	if(  line->tex_ceil && line->h1==-128 && line->h2==-128 ) cm= 29;	//	Ceil 128 128  CLIPPED
	if( !line->tex_ceil && line->h1==-128 && line->h2==-128 ) fm= 30;	//	Sky  128 128  front
	if( !line->tex_ceil && line->h1==-128 && line->h2==-128 ) cm= 31;	//	Sky  128 128  CLIPPED
	if( !line->tex_ceil && line->h1== -96 && line->h2== -96 ) fm= 32;	//	Sky   96  96  front
	if( !line->tex_ceil && line->h1== -96 && line->h2== -96 ) cm= 33;	//	Sky   96  96  CLIPPED
	if( !line->tex_ceil && line->h1== -64 && line->h2== -64 ) fm= 34;	//	Sky   64  64  front
	if( !line->tex_ceil && line->h1== -64 && line->h2== -64 ) cm= 35;	//	Sky   64  64  CLIPPED
