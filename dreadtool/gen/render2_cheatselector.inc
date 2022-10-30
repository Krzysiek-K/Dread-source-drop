	if(  7<cost && !is_two_sided && !has_sky && !draw_lower ) mode=  0, cost=7;	//	Ceil y1 Upper y2 Floor
	if( 10<cost && !is_two_sided && !has_sky &&  draw_lower ) mode=  1, cost=10;	//	Ceil y1 Upper y2 Lower y4 Floor
	if(  7<cost && !is_two_sided &&  has_sky && !draw_lower ) mode=  2, cost=7;	//	Sky y1 Upper y2 Floor
	if( 10<cost && !is_two_sided &&  has_sky &&  draw_lower ) mode=  3, cost=10;	//	Sky y1 Upper y2 Lower y4 Floor
	if(  3<cost &&  is_two_sided && !draw_ceiling && !draw_upper && !draw_lower ) mode=  4, cost=3;	//	gap y3 Floor
	if(  6<cost &&  is_two_sided && !draw_ceiling && !draw_upper ) mode=  5, cost=6;	//	gap y3 Lower y4 Floor
	if(  3<cost &&  is_two_sided && !has_sky && !draw_upper && !draw_lower && !draw_floor ) mode=  6, cost=3;	//	Ceil y1 gap
	if(  5<cost &&  is_two_sided && !has_sky && !draw_upper && !draw_lower ) mode=  7, cost=5;	//	Ceil y1 gap y3 Floor
	if(  8<cost &&  is_two_sided && !has_sky && !draw_upper ) mode=  8, cost=8;	//	Ceil y1 gap y3 Lower y4 Floor
	if(  6<cost &&  is_two_sided && !has_sky && !draw_lower && !draw_floor ) mode=  9, cost=6;	//	Ceil y1 Upper y2 gap
	if(  8<cost &&  is_two_sided && !has_sky && !draw_lower ) mode= 10, cost=8;	//	Ceil y1 Upper y2 gap y3 Floor
	if( 11<cost &&  is_two_sided && !has_sky ) mode= 11, cost=11;	//	Ceil y1 Upper y2 gap y3 Lower y4 Floor
	if(  3<cost &&  is_two_sided &&  has_sky && !draw_upper && !draw_lower && !draw_floor ) mode= 12, cost=3;	//	Sky y1 gap
	if(  5<cost &&  is_two_sided &&  has_sky && !draw_upper && !draw_lower ) mode= 13, cost=5;	//	Sky y1 gap y3 Floor
	if(  8<cost &&  is_two_sided &&  has_sky && !draw_upper ) mode= 14, cost=8;	//	Sky y1 gap y3 Lower y4 Floor
	if(  6<cost &&  is_two_sided &&  has_sky && !draw_lower && !draw_floor ) mode= 15, cost=6;	//	Sky y1 Upper y2 gap
	if(  8<cost &&  is_two_sided &&  has_sky && !draw_lower ) mode= 16, cost=8;	//	Sky y1 Upper y2 gap y3 Floor
	if( 11<cost &&  is_two_sided &&  has_sky ) mode= 17, cost=11;	//	Sky y1 Upper y2 gap y3 Lower y4 Floor
