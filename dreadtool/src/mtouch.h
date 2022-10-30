
#pragma once



class MultiTouch {
public:

    MultiTouch() { Clear(); }

    void DisableUntilFrame() { disable_until_frame = true; }

    void OnAfterFrame();
    void TouchUpdate(int con,int seq,int xpos,int ypos,bool down);

    vec2 SenseDelta(void *user,const vec2 &bmin,const vec2 &bmax) {
        return vec2( GetMotion(user,bmin,bmax,GM_DELTA_X,0.f),
                    GetMotion(user,bmin,bmax,GM_DELTA_Y,0.f) );
    }

    vec2 SenseLocalPos(void *user,const vec2 &bmin,const vec2 &bmax) {
		return vec2( GetMotion(user,bmin,bmax,GM_LOCAL_X,.5f),
					GetMotion(user,bmin,bmax,GM_LOCAL_Y,.5f) );
    }

	vec2 SenseLocalPos(void *user, const vec2 &bmin, const vec2 &bmax, const vec2 &def) {
		return vec2(GetMotion(user, bmin, bmax, GM_LOCAL_X, def.x),
			GetMotion(user, bmin, bmax, GM_LOCAL_Y, def.y));
	}
	
	vec2 SenseDragPos(void *user,const vec2 &bmin,const vec2 &bmax) {
		return vec2( GetMotion(user,bmin,bmax,GM_DRAG_X,.5f),
					GetMotion(user,bmin,bmax,GM_DRAG_Y,.5f) );
    }

	float SenseSliderX(void *user,const vec2 &bmin,const vec2 &bmax,float def)    {  return GetMotion(user,bmin,bmax,GM_TOUCH_SLIDER_X,def); }
    float SenseSliderY(void *user,const vec2 &bmin,const vec2 &bmax,float def)    {  return GetMotion(user,bmin,bmax,GM_TOUCH_SLIDER_Y,def); }

    bool SenseActive(void *user,const vec2 &bmin,const vec2 &bmax)        {   return GetMotion(user,bmin,bmax,GM_ACTIVE,0.f) != 0.f;      }
    bool SenseScavenge(void *user,const vec2 &bmin,const vec2 &bmax)      {   return GetMotion(user,bmin,bmax,GM_SCAVENGE,0.f) != 0.f;        }
	bool SenseClick(void *user,const vec2 &bmin,const vec2 &bmax)         {   return GetMotion(user,bmin,bmax,GM_TOUCH_CLICK,0.f) != 0.f;     }
    bool SenseHold(void *user,const vec2 &bmin,const vec2 &bmax)          {   return GetMotion(user,bmin,bmax,GM_TOUCH_HOLD,0.f) != 0.f;      }
    bool SenseFullButton(void *user,const vec2 &bmin,const vec2 &bmax)    {   return GetMotion(user,bmin,bmax,GM_TOUCH_BT_PRESS,0.f) > 0;  }
    bool SenseHalfButton(void *user,const vec2 &bmin,const vec2 &bmax)    {   return GetMotion(user,bmin,bmax,GM_TOUCH_BT_HOLD,0.f) > .1f;   }

	int TrasnferOwnership(void *from,void *to);

private:
    enum { MAX_TOUCH = 16 };

    enum {
	    GM_DELTA_X, 
	    GM_DELTA_Y,   
	    GM_LOCAL_X,   
	    GM_LOCAL_Y,   
	    GM_DRAG_X,
	    GM_DRAG_Y,
	    GM_ACTIVE,
	    GM_SCAVENGE,
	    GM_TOUCH_CLICK,
	    GM_TOUCH_HOLD,
        GM_TOUCH_HOLD_SHARED,
        GM_TOUCH_BT_PRESS,
	    GM_TOUCH_BT_HOLD,
	    GM_TOUCH_SLIDER_X,
	    GM_TOUCH_SLIDER_Y,
    };

    struct TouchStruct {
	    int		_contact_id;
	    int		_seq_id;
	    vec2		curpos;
	    vec2		prevpos;
	    vec2		startpos;
	    vec2		delta;
        vec2     moved;
	    void	*user_binding;
	    int		last_frame;
	    bool	is_click;
	    bool	is_touching;
	    bool	is_leaving;
	    bool	is_active;
        float   duration;
    };

    TouchStruct touch[MAX_TOUCH];
    bool        disable_until_frame;
    int         frame_num;

    void  Clear();
    float GetMotion(void *user,const vec2 &bmin,const vec2 &bmax,int type,float def);
};


extern MultiTouch mt;
