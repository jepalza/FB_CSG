



int bb_encloses(double *a, double *b);
int bb_apart(double *a, double *b);
int bb_empty(double *a);
void bb_normalize(double *bbox);

void bb_expand(double *bbox, double *v);
void bb_expand_box(double *bbox, double *bb);


struct BBLIST
{
	double bb[6];
	struct BBLIST *next;
	
};

void bb_match_resolution_expand(struct BBLIST **head, double resolution);

void bb_list_add(struct BBLIST **head, double *bb);
void bb_list_remove(struct BBLIST **head, struct BBLIST *item);
void bb_list_free(struct BBLIST **head);

void bb_transform_bbox(double *bbox, double *sbbox, double *m);
void bb_add(double *bbox, double *sbbox);
int bb_point_inside(double *bbox, double *v);



int bb_list_next_lowest(struct BBLIST **bl, int dimension, double y, double *returned_start, double *returned_stop);
int bb_list_dim_test(struct BBLIST **head, int dimension, double y);
void bb_list_extents(struct BBLIST **head, double *bbox);