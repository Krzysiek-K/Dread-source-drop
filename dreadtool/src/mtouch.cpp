
#include "common.h"


MultiTouch mt;


/*


* GM_DELTA_X/Y:

    user:       grabs unused touches on click within the box        [grabOnClick]
                then returns delta value
    no user:    returns delta if curpos & prevpos is within box     [nullReqBox + nullReqPrevBox]

* GM_LOCAL_X/Y:

    user:       grabs unused touches on click within the box        [grabOnClick]
                then returns clamped local coords
    no user:    returns coords if curpos within box                 [nullReqBox]

* GM_SCAVENGE:

    user:       grabs unused touches within box (no click needed)   [grabOnBox]
                returns 1 if just grabbed
    no user:    no operation

* GM_TOUCH_CLICK:

    user:       grabs unused touches on click within the box    [grabOnClick]
                then returns 1 on click
    no user:    returns 1 on click within the box               [nullReqBox]

* GM_TOUCH_HOLD:

    user:       grabs unused touches on click within the box    [grabOnClick]
                then returns duration when pressed
    no user:    returns duration when pressed within the box    [nullReqBox]

* GM_TOUCH_HOLD_SHARED:

    user:       grabs unused touches on click within the box    [grabOnClick]
                then returns duration when pressed in the box   [muteOutside]
    no user:    returns duration when pressed within the box    [nullReqBox]

* GM_TOUCH_BT_PRESS:

    user:       grabs unused touches on click within the box                [grabOnClick]
                frees touch on move                                         [freeOnMoveX + freeOnMoveY]
                touch must end within box                                   [muteOutside]
                on release returns number of seconds the touch was held
    no user:    when never moved or used touch that started in the box releases within the box -> returns number of seconds     [nullNoMoveX + nullNoMoveY + nullReqStartBox + nullReqBox]

* GM_TOUCH_BT_HOLD:

    user:       grabs unused touches on click within the box                [grabOnClick]
                frees touch on move                                         [freeOnMoveX + freeOnMoveY]
                returns 1 as long as touch is held in the box               [muteOutside]
    no user:    returns 1 as long as never moved or used touch that started in the box is within the box    [nullNoMoveX + nullNoMoveY + nullReqStartBox + nullReqBox]

* GM_TOUCH_SLIDER_X/Y:

    user:       grabs unused touches on click within the box                        [grabOnClick]
                frees touch on move in wrong direction                              [freeOnMoveY/X]
                after touch is moved in right direction starts returning LOCAL_X/Y  [silenceNotMovedX/Y]
    no user:    same as LOCAL_X/Y                                                   [nullReqBox]



    with user:
        grabOnClick         - grabs unused touches on click within the box
        grabOnBox           - grabs unused touches within the box (no click required)
        freeOnMoveX/Y       - frees touch if moved in X/Y direction (unless other direction is allowed and already moved)
        muteOutside         - return 0 if touch is currently outside bounds

    no user:
        nullReqBox          - requires curpos is within box
        nullReqPrevBox      - requires prevpos is within box
        nullReqStartBox     - requires start is within box
        nullNoMoveX/Y       - requires that touch isn't moved in X/Y direction

    all:
        allowLeaving        - doesn't cull touches that are leaving and no longer down
        silenceNotMovedX/Y  - don't affect result (but don't release) if not moved in X/Y direction
        dontSilenceLeaving  = disable silencing if touch is leaving

*/




void MultiTouch::Clear()
{
    memset(touch,0,sizeof(touch));
    disable_until_frame = false;
    frame_num = 0;
}


float MultiTouch::GetMotion(void *user,const vec2 &bmin,const vec2 &bmax,int type,float def)
{
	if(disable_until_frame)
		return def;

    // seek modes
    enum {
        X  = 1,
        Y  = 2,
        XY = X | Y
    };

    // user
    bool grabOnClick = false;
    bool grabOnBox = false;
    int  freeOnMove = 0;
    bool muteOutside = false;
    // no user
    bool nullReqBox = false;
    bool nullReqPrevBox = false;
    bool nullReqStartBox = false;
    int  nullNoMove = 0;
    // all
    bool allowLeaving = false;
    int  silenceNotMoved = 0;
    bool dontSilenceLeaving = false;;

    // geometry
	vec2 dim = bmax - bmin;
    float _static_dist = (dim.x<dim.y ? dim.x : dim.y)*.3f;
    float static_dist_x = _static_dist;
    float static_dist_y = _static_dist;

    // init seek modes
    switch(type)
    {
    case GM_DELTA_X:
    case GM_DELTA_Y:
        grabOnClick = true;
        nullReqBox = true;
        nullReqPrevBox = true;
        break;

    case GM_LOCAL_X:
    case GM_LOCAL_Y:
    case GM_TOUCH_CLICK:
    case GM_TOUCH_HOLD:
        grabOnClick = true;
        nullReqBox = true;
        break;

    case GM_DRAG_X:
    case GM_DRAG_Y:
    case GM_ACTIVE:
        break;

	case GM_TOUCH_HOLD_SHARED:
        muteOutside = true;
        grabOnClick = true;
        nullReqBox = true;
        break;

    case GM_SCAVENGE:
        grabOnBox = false;
        break;

    case GM_TOUCH_BT_PRESS:
    case GM_TOUCH_BT_HOLD:
        grabOnClick = true;
        freeOnMove = XY;
        nullNoMove = XY;
        nullReqStartBox = true;
        nullReqBox = true;
        muteOutside = true;
        allowLeaving = (type==GM_TOUCH_BT_PRESS);
        break;
    
    case GM_TOUCH_SLIDER_X:
        grabOnClick         = true;
        freeOnMove          = Y;
        silenceNotMoved     = X;
        nullReqBox          = true;
        static_dist_x       *= .5f;
        allowLeaving        = true;
        dontSilenceLeaving  = true;
        break;

    case GM_TOUCH_SLIDER_Y:
        grabOnClick         = true;
        freeOnMove          = X;
        silenceNotMoved     = Y;
        nullReqBox          = true;
        static_dist_y       *= .5f;
        allowLeaving        = true;
        dontSilenceLeaving  = true;
        break;

    default:
        return def;
    }

    // touch loop
    float v = def;
    for(int i=0;i<MAX_TOUCH;i++)
    {
        TouchStruct &t = touch[i];
        bool justGrabbed = false;

        // skip inactive
        if(!t.is_active) continue;

        // skip released - unless button event
		if(!t.is_touching && !allowLeaving)
			continue;

        // check boxes
        bool boxStart   = t.startpos.in_box(bmin,bmax);
        bool boxPrev    = t.prevpos.in_box(bmin,bmax);
        bool boxNow     = t.curpos.in_box(bmin,bmax);

        // grabOnClick
        if(user && grabOnClick)
            if( !t.user_binding && t.is_click && boxNow )
            {
                t.user_binding = user;
                justGrabbed = true;
            }

        // grabOnBox
        if(user && grabOnClick)
            if( !t.user_binding && boxNow )
            {
                t.user_binding = user;
                justGrabbed = true;
            }

        // freeOnMove
        bool movedX = (t.moved.x > static_dist_x);
        bool movedY = (t.moved.y > static_dist_y);
        int movBits = (movedX ? X : 0) | (movedY ? Y : 0);
        if(user && freeOnMove && t.user_binding==user)
        {
            bool movedBad  = (movBits& freeOnMove)!=0;
            bool movedGood = (movBits&~freeOnMove)!=0;

            if( movedBad && !movedGood )
                t.user_binding = NULL;
        }

        if(!user)
        {
            if( nullReqBox      && !boxNow   ) continue;
            if( nullReqPrevBox  && !boxPrev  ) continue;
            if( nullReqStartBox && !boxStart ) continue;
            if( nullNoMove && (nullNoMove & movBits)!=0 ) continue;
        }

        // cull users
        if(t.user_binding != user)
            continue;

        // silenceNotMoved
        if( silenceNotMoved && (silenceNotMoved & movBits)==0 )
            if( !t.is_leaving || !dontSilenceLeaving )
                continue;

        // muteOutside
        if( muteOutside && !boxNow )
            continue;

        // compute value
		if(type==GM_DELTA_X			) { v += t.delta.x; t.delta.x = 0; }
		if(type==GM_DELTA_Y			) { v += t.delta.y; t.delta.y = 0; }
        if(type==GM_LOCAL_X			) { v = (t.curpos.x-bmin.x)/(bmax.x-bmin.x); if(v<0) v=0; if(v>1) v=1; }
		if(type==GM_LOCAL_Y			) { v = (t.curpos.y-bmin.y)/(bmax.y-bmin.y); if(v<0) v=0; if(v>1) v=1; }
        if(type==GM_DRAG_X			) { v = (t.curpos.x-bmin.x)/(bmax.x-bmin.x); if(v<0) v=0; if(v>1) v=1; }
		if(type==GM_DRAG_Y			) { v = (t.curpos.y-bmin.y)/(bmax.y-bmin.y); if(v<0) v=0; if(v>1) v=1; }
		if(type==GM_ACTIVE			) { v = 1; }
		if(type==GM_SCAVENGE        ) { if(justGrabbed) v = 1.f; }
		if(type==GM_TOUCH_CLICK		) { if(t.is_click) v = 1.f; t.is_click = false; }
        if(type==GM_TOUCH_HOLD		) v = t.duration;
        if(type==GM_TOUCH_BT_PRESS	) { if(t.is_leaving && v<t.duration) v = t.duration; t.is_leaving=false; }
		if(type==GM_TOUCH_BT_HOLD	) v = t.duration;
        if(type==GM_TOUCH_SLIDER_X	) { v = (t.curpos.x-bmin.x)/(bmax.x-bmin.x); if(v<0) v=0; if(v>1) v=1; }
		if(type==GM_TOUCH_SLIDER_Y  ) { v = (t.curpos.y-bmin.y)/(bmax.y-bmin.y); if(v<0) v=0; if(v>1) v=1; }
    }

    return v;
}

void MultiTouch::OnAfterFrame()
{
    for(int i=0;i<MAX_TOUCH;i++)
	{
		TouchStruct &t = touch[i];
		if(!t.is_active) continue;

		t.is_click = false;
		t.delta = vec2(0,0);
		
		if(!t.is_touching)
		{
			t.is_active = false;
			t.user_binding = NULL;
		}
	}

    frame_num++;
}

void MultiTouch::TouchUpdate(int con,int seq,int xpos,int ypos,bool down)
{
	TouchStruct *t = NULL;
	vec2 pos(xpos,ypos);

	for(int i=0;i<MAX_TOUCH;i++)
		if(touch[i].is_active && touch[i]._contact_id==con && touch[i]._seq_id==seq)
		{
			t = &touch[i];
			break;
		}

	if(!t && !down)
		return;

	if(!t)
		for(int i=0;i<MAX_TOUCH;i++)
			if(!touch[i].is_active)
			{
				t = &touch[i];
				t->_contact_id = con;
				t->_seq_id = seq;
				t->curpos = pos;
				t->prevpos = pos;
				t->startpos = pos;
				t->delta = vec2(0,0);
				t->moved = vec2(0,0);
				t->user_binding = NULL;
				t->last_frame = frame_num;
				t->is_click = false;
				t->is_touching = false;
				t->is_leaving = false;
				t->is_active = true;
                t->duration = 0.001f;
				break;
			}

	if(!t)
		return;

	if(!down && xpos==-1 && ypos==-1)
		pos = t->curpos;

    // update position
	t->last_frame = frame_num;
	t->prevpos = t->curpos;
	if(t->is_touching || down)
		t->curpos = pos;

    // update delta
	t->delta += t->curpos - t->prevpos;

    // update moved
    vec2 ds = t->curpos - t->startpos;
    if(ds.x<0) ds.x=-ds.x;
    if(ds.y<0) ds.y=-ds.y;
    if(ds.x>t->moved.x) t->moved.x=ds.x;
    if(ds.y>t->moved.y) t->moved.y=ds.y;

    // update touching & active
	if(down && !t->is_touching)
		t->is_click = true;
	if(t->is_touching && !down)
		t->is_leaving = true;
	t->is_touching = down;
	t->is_active = true;
    t->duration += Dev.GetTimeDelta();
}

int MultiTouch::TrasnferOwnership(void *from,void *to)
{
	int cnt = 0;
	for(int i=0;i<MAX_TOUCH;i++)
		if(touch[i].is_active && touch[i].user_binding==from)
		{
			touch[i].user_binding = to;
			cnt++;
		}

	return cnt;
}
