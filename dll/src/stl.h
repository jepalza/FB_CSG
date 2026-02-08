struct MARCH_THREAD_DATA
{
	double x;
	double y;
	double zstart;
	double zend;
	double resolution;
	int action;//set to -1 to kill thread, 0 to indicate consumed
};

#pragma pack(push, 1)
struct STL
{
	float n[3];
	float v1[3];
	float v2[3];
	float v3[3];
	uint16_t attr;
};
#pragma pack(pop)

void csg_save_stl(char *file, double resolution, int threads);