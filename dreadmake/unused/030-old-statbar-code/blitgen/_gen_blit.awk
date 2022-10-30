BEGIN{
	print "";
}



function field(v,l)
{
	printf("%s",v);
	l -= length(v);
	while(l>0) { printf("\t"); l -= 4; }
}


function instr(i,arg,cmt)
{
	printf("\t");
	field(i,8);
	if(cmt)
	{
		field(arg,4*9);
		printf("; %s",cmt);
	}
	else
		printf("%s",arg);
	print "";
}


function move_w(r1,v1,	vx)
{
	vx = (v1~/^0x/) ? sprintf("$%04X",strtonum(v1)) : v1;
	instr("move.w", "#" vx ", " r1 "(a1)", r1 " = " v1);
}

function move_l(r1,v1)
{
		 if(v1~/^d[0-7]/)	instr("move.l", v1 ", " r1 "(a1)", r1 " = " v1);
	else if(v1~/^a[0-7]/)	instr("move.l", v1 ", " r1 "(a1)", r1 " = " v1);
	else					instr("move.l", "#" v1 ", " r1 "(a1)", r1 " = " v1);
}

function move_dual(r1,v1,r2,v2)
{
	#move_w(r1,v1);
	#move_w(r2,v2);
	instr("move.l", sprintf("#$%04X%04X, %s(a1)",and(strtonum(v1),0xFFFF),and(strtonum(v2),0xFFFF),r1), r1 " = " v1);
	instr("", "", r2 " = " v2);
}


#
#	generic stat blit:
#
#		BLTCON0 + BLTCON1		$________
#		BLTAFWM + BLTALWM		$________
#		BLTADAT					$FFFF
#		BLTCMOD + BLTBMOD		$xxxxyyyy
#		BLTAMOD + BLTDMOD		$yyyyxxxx			[word-swapped version of the above]
#		BLTAPT					$<source> + offs
#		BLTBPT					$<source>
#		BLTCPT					$<target>
#		BLTDPT					$<target>
#		BLTSIZE					$____
#
#
#			dword	bltconx;
#			dword	bltawm;
#			dword	bltxmod;
#			word	*source;					// runtime
#			word	srcmask_offs;
#			word	target_offs;
#			word	bltsize;
#			void	*blitstruct_chain;			// runtime
#


#
#	Stat_Clear:
#
#		BLTCON0 + BLTCON1		$07CA0000
#		BLTAFWM + BLTALWM							[compute]
#		BLTADAT					$FFFF
#		BLTCMOD + BLTBMOD		40-2ww | 40-2ww		[compute]
#				  BLTDMOD				 40-2ww		[compute]
#		
#		BLTBPT					[source]
#		BLTCPT					[target]
#		BLTDPT					[target]
#		BLTSIZE					[compute]
#
function Stat_Clear(x0, y0, w, h,		x2, ww, off)
{
	printf("\t\t\t\t\t\t\t\t\t\t\t\t; Stat_Clear( %d, %d, %d, %d );\n",x0,y0,w,h);

	x2 = x0 + w;
	ww = rshift((x2+15),4) - rshift(x0,4);

	move_dual(	"BLTCON0","0x07CA",
				"BLTCON1","0x0000");
	move_dual(	"BLTAFWM", sprintf("0x%04X",rshift(0xFFFF,and(x0,15))),
				"BLTALWM", sprintf("0x%04X",and(x2,15) ? and(lshift(0xFFFF,16-and(x2,15)),0xFFFF) : 0xFFFF));
	move_w(		"BLTADAT", "0xFFFF" );
	move_dual(	"BLTCMOD", 40-2*ww,
				"BLTBMOD", 40-2*ww );
	move_w(		"BLTDMOD", 40-2*ww );

	off = 2*(rshift(x0,4) + 20*4*y0);

	move_l(		"BLTBPT", "_GFX_STBAR0 + " (off+6) );

	instr("move.l","_statbar_buffer, d0","d0 = _statbar_buffer + " off);
	instr("add.l","#" off ", d0");
	move_l(		"BLTCPT", "d0" );
	move_l(		"BLTDPT", "d0" );
	move_w(		"BLTSIZE", sprintf("((%d<<6) | %d)",h*4,ww) );
}


#
#	Stat_Draw:
#
#		BLTCON0 + BLTCON1		$_FCA_000		[compute '_']
#		BLTAFWM + BLTALWM		$FFFF____		[compute '_', $FFFF or $0000]
#
#		BLTCMOD + BLTBMOD		40-2ww | smod	[compute]
#		BLTAMOD + BLTDMOD		smod | 40-2ww	[compute]
#		BLTAPT					[source + mask offset]
#		BLTBPT					[source]
#		BLTCPT					[target]
#		BLTDPT					[target]
#		BLTSIZE					[compute]
#
function Stat_Draw(x0, y0, w, h,		x2, ww, off, smod)
{
	printf("\t\t\t\t\t\t\t\t\t\t\t\t; Stat_Draw( %d, %d, %d, %d );\n",x0,y0,w,h);

	x2 = x0 + w;
	ww = rshift((x2+15),4) - rshift(x0,4);

	move_dual(	"BLTCON0",sprintf("0x%04X",0x0FCA + and(x0,15)*0x1000),
				"BLTCON1",sprintf("0x%04X",0x0000 + and(x0,15)*0x1000));

	move_dual(	"BLTAFWM", "0xFFFF",
				"BLTALWM", ww>rshift(w+15,4) ? "0x0000" : "0xFFFF");

	smod = -2*(ww-rshift(w+15,4));
	move_dual(	"BLTCMOD", 40-2*ww,
				"BLTBMOD", smod );
	move_dual(	"BLTAMOD", smod,
				"BLTDMOD", 40-2*ww );

	move_l(		"BLTBPT", "a0" );
	instr("add.l",sprintf("#%d, a0",2*4*rshift(w+15,4)*h));
	move_l(		"BLTAPT", "a0" );

	off = 2*(rshift(x0,4) + 20*4*y0);
	instr("move.l","_statbar_buffer, d0","d0 = _statbar_buffer + " off);
	instr("add.l","#" off ", d0");
	move_l(		"BLTCPT", "d0" );
	move_l(		"BLTDPT", "d0" );
	move_w(		"BLTSIZE", sprintf("((%d<<6) | %d)",h*4,ww) );
}

function Make_End(nxt)
{
	if( nxt != ".end" )
	{
		instr("lea.l", nxt "(pc), a0");
		instr("move.l", "a0, _Blit_IRQ_fn");
	}
	else
	{
		if( !END_DONE )
		{
			print ".end:";
			END_DONE = 1;
		}
		instr("moveq.l", "#0, d0");
		instr("move.l", "d0, _Blit_IRQ_fn");
	}
	instr("rts");
}

function Make_Check(field, nxt)
{
	print ".check_" field ":";
	instr("move.w",	"_e_status_bar+statbar_" field ", d0","check " field);
	instr("cmp.w",	"_e_status_bar_shadow+statbar_" field ", d0");
	instr("beq", nxt);
}

function Make_Draw(field, nxt)
{
	print ".draw_" field ":"
	instr("move.w",	"_e_status_bar+statbar_" field ", d0", "draw " field);
	instr("move.w",	"d0, _e_status_bar_shadow+statbar_" field);
	instr("beq",	nxt);
	instr("lsl.w",	"#2, d0");
	instr("lea.l",	"_STAT_DIGITS, a0");
	instr("move.l",	"(a0,d0.w), a0");
}

function Make_Render(chain,	i,j,xmin,xmax,nxt)
{
	# Checks / clears
	for( i = 0; i<NFIELDS; i++ )
	{
		Make_Check(FIELD[i], i+1<NFIELDS ? ".check_" FIELD[i+1] : chain);
		print "";
		printf("\t; Clear: ");
		xmin=XPOS[i];
		xmax=XPOS[i]+WIDTH;
		for(j=i;j<NFIELDS;j++)
		{
			if(j>i) printf(", ");
			printf("%s",FIELD[j]);

			if( XPOS[j]<xmin ) xmin=XPOS[j];
			if( XPOS[j]+WIDTH>xmax ) xmax=XPOS[j]+WIDTH;
		}
		print "";
		Stat_Clear(xmin,Y0,xmax-xmin,HEIGHT);
		print "";
		Make_End(".draw_" FIELD[i]);
		print "";
		print "";
	}

	# Draws
	for( i = 0; i<NFIELDS; i++ )
	{
		nxt = i+1 < NFIELDS ? ".draw_" FIELD[i+1] : chain;
		Make_Draw(FIELD[i], nxt);
		print "";
		Stat_Draw(XPOS[i],Y0,WIDTH,HEIGHT);
		print "";
		Make_End(nxt);
		if( nxt != ".end" )
		{
			print "";
			print "";
		}
	}
}

function weapon_stat(name,x0,y0,nxt)
{
	NFIELDS = 1;
	WIDTH = 4;
	HEIGHT = 6;
	Y0 = y0;
	FIELD[0] = name;		XPOS[0] = x0;
	Make_Render(nxt);
}

function ammo_stat(name,x0,y0,nxt)
{
	NFIELDS = 3;
	WIDTH = 4;
	HEIGHT = 6;
	Y0 = y0;
	FIELD[0] = name "_100";		XPOS[0] = x0-4-4;
	FIELD[1] = name "_10";		XPOS[1] = x0-4;
	FIELD[2] = name "_1";		XPOS[2] = x0;
	Make_Render(nxt);
}

function key_stat(name,x0,y0,nxt)
{
	NFIELDS = 1;
	WIDTH = 7;
	HEIGHT = 5;
	Y0 = y0;
	FIELD[0] = name;		XPOS[0] = x0;
	Make_Render(nxt);
}

END{
	#Stat_Clear(4, 4, 13*3, 16, 0);
	#Stat_Draw(4, 4, 13, 16);

	#	Ammo		4	17	30				Y=4		(step 13)
	NFIELDS = 3;
	WIDTH = 13;
	HEIGHT = 16;
	Y0 = 4;
	FIELD[0] = "ammo_100";	XPOS[0] = 4;
	FIELD[1] = "ammo_10";	XPOS[1] = 4+13;
	FIELD[2] = "ammo_1";	XPOS[2] = 4+13+13;
	Make_Render(".check_health_prc");

	#	Health		51	64	77	90			Y=4		(step 13)
	NFIELDS = 4;
	WIDTH = 13;
	HEIGHT = 16;
	Y0 = 4;
	FIELD[0] = "health_prc";	XPOS[0] = 51+13+13+13;
	FIELD[1] = "health_100";	XPOS[1] = 51;
	FIELD[2] = "health_10";		XPOS[2] = 51+13;
	FIELD[3] = "health_1";		XPOS[3] = 51+13+13;
	Make_Render(".check_armor_prc");

	#	Armor		182	195 208 221			Y=4		(step 13)
	NFIELDS = 4;
	WIDTH = 13;
	HEIGHT = 16;
	Y0 = 4;
	FIELD[0] = "armor_prc";		XPOS[0] = 182+13+13+13;
	FIELD[1] = "armor_100";		XPOS[1] = 182;
	FIELD[2] = "armor_10";		XPOS[2] = 182+13;
	FIELD[3] = "armor_1";		XPOS[3] = 182+13+13;
	Make_Render(".check_bullets_100");

	#	Ammo stats
	ammo_stat("bullets",		287, 5,".check_shells_100");
	ammo_stat("shells",			287,11,".check_rockets_100");
	ammo_stat("rockets",		287,17,".check_cells_100");
	ammo_stat("cells",			287,23,".check_cap_bullets_100");
	ammo_stat("cap_bullets",	310, 5,".check_cap_shells_100");
	ammo_stat("cap_shells",		310,11,".check_cap_rockets_100");
	ammo_stat("cap_rockets",	310,17,".check_cap_cells_100");
	ammo_stat("cap_cells",		310,23,".check_key_0");

	# Keys
	key_stat("key_0",239, 3,".check_key_1");
	key_stat("key_1",239,13,".check_key_2");
	key_stat("key_2",239,23,".check_weapon_0");

	# Weapons
	weapon_stat("weapon_0",111, 4,".check_weapon_1");
	weapon_stat("weapon_1",123, 4,".check_weapon_2");
	weapon_stat("weapon_2",135, 4,".check_weapon_3");
	weapon_stat("weapon_3",111,14,".check_weapon_4");
	weapon_stat("weapon_4",123,14,".check_weapon_5");
	weapon_stat("weapon_5",135,14,".end");

}
