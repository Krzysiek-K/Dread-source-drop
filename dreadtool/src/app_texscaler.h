
#pragma once


class TexScalerApp : public SubApplication {
public:

	DataTexture				screen;

	DataTexture				tex;
	TexScalerTester			renderer;
	int						bank_select;
	int						draw_mode;
	float					wall_distance;
	float					wall_angle;
	float					wall_scroll;

	TexScalerMethodBank		method_basic_clear;
	TexScalerMethodBank		method_basic_avoid;
	TexScalerMethodBank		method_basic_masked;
	TexScalerMethodBank		method_shift_clear;
	TexScalerMethodBank		method_shift_avoid;
	TexScalerMethodBank		method_shift_masked;

	vector<TexScalerMethodBank*>	method_banks;
	int								current_bank;

	virtual const char *GetName() { return "Tex Scaler"; }

	virtual void Init();
	virtual void Shutdown();

	void SetScreenSize(int w, int h);

	void DrawColumn(int x, int tx, TexScalerMethod &m);

	virtual void Run();
	virtual void RunMenu();

protected:
	void InitBank(TexScalerMethodBank &m,const char *name,const char *ser);

	TexScalerMethodBank *GetCurentBank() { return current_bank>=0 && current_bank<(int)method_banks.size() ? method_banks[current_bank] : NULL; }
};
