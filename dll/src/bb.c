/* 
Copyright (C) 2018 Cyrus Welgan 
cyrus@c-csg.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#include "c-csg.h"
#include "bb.h"


struct RBOX
{
	double bb[6];
	int valid;
};

void bb_normalize(double *bbox)
{
	//keep lower values on the left and bottom
	double s;

	if(bbox[0] > bbox[3])
		{
			s = bbox[0];
			bbox[0] = bbox[3];
			bbox[3] = s;
		}

	if(bbox[1] > bbox[4])
		{
			s = bbox[1];
			bbox[1] = bbox[4];
			bbox[4] = s;
		}

	if(bbox[2] > bbox[5])
		{
			s = bbox[2];
			bbox[2] = bbox[5];
			bbox[5] = s;
		}
}

void bb_expand(double *bbox, double *v)
{
	//expand bbox to include point
	if(v[0] < bbox[0])
		{
			bbox[0] = v[0];
		}
	else if(v[0] > bbox[3])
		{
			bbox[3] = v[0];
		}

	if(v[1] < bbox[1])
		{
			bbox[1] = v[1];
		}
	else if(v[1] > bbox[4])
		{
			bbox[4] = v[1];
		}

	if(v[2] < bbox[2])
		{
			bbox[2] = v[2];
		}
	else if(v[2] > bbox[5])
		{
			bbox[5] = v[2];
		}
}

void bb_transform_bbox(double *bbox, double *sbbox, double *m)
{
	//transform 8 corner points, set up new bbox
	//012, 015, 042, 045
	//312, 315, 342, 345
	double a[3];
	double b[3];
	double c[3];
	a[0] = sbbox[0];
	a[1] = sbbox[1];
	a[2] = sbbox[2];
	m_transform(b, a, m);
	a[0] = sbbox[3];
	a[1] = sbbox[4];
	a[2] = sbbox[5];
	m_transform(c, a, m);

	if(b[0] < c[0])
		{
			bbox[0] = b[0];
			bbox[3] = c[0];
		}
	else
		{
			bbox[3] = b[0];
			bbox[0] = c[0];
		}

	if(b[1] < c[1])
		{
			bbox[1] = b[1];
			bbox[4] = c[1];
		}
	else
		{
			bbox[4] = b[1];
			bbox[1] = c[1];
		}

	if(b[2] < c[2])
		{
			bbox[2] = b[2];
			bbox[5] = c[2];
		}
	else
		{
			bbox[5] = b[2];
			bbox[2] = c[2];
		}

	a[0] = sbbox[0];
	a[1] = sbbox[4];
	a[2] = sbbox[5];
	m_transform(b, a, m);
	bb_expand(bbox, b);
	a[0] = sbbox[3];
	a[1] = sbbox[1];
	a[2] = sbbox[2];
	m_transform(b, a, m);
	bb_expand(bbox, b);
	a[0] = sbbox[3];
	a[1] = sbbox[4];
	a[2] = sbbox[2];
	m_transform(b, a, m);
	bb_expand(bbox, b);
	a[0] = sbbox[0];
	a[1] = sbbox[4];
	a[2] = sbbox[2];
	m_transform(b, a, m);
	bb_expand(bbox, b);
	a[0] = sbbox[0];
	a[1] = sbbox[1];
	a[2] = sbbox[5];
	m_transform(b, a, m);
	bb_expand(bbox, b);
	a[0] = sbbox[3];
	a[1] = sbbox[1];
	a[2] = sbbox[5];
	m_transform(b, a, m);
	bb_expand(bbox, b);
}

void bb_add(double *bbox, double *sbbox)
{
	//extend extents of bbox if necessary to contain sbbox
	bb_expand(bbox, &sbbox[0]);
	bb_expand(bbox, &sbbox[3]);
}

inline int bb_point_inside(double *bbox, double *v)
{
	return (
	           v[0] >= bbox[0] && v[0] <= bbox[3] &&
	           v[1] >= bbox[1] && v[1] <= bbox[4] &&
	           v[2] >= bbox[2] && v[2] <= bbox[5]
	       );
}






//does a enclose b?
int bb_encloses(double *a, double *b)
{
	if(a[0] <= b[0] && a[3] >= b[3] &&
	        a[1] <= b[1] && a[4] >= b[4] &&
	        a[2] <= b[2] && a[5] >= b[5])
		return 1;

	return 0;
}

//is a apart from b?
int bb_apart(double *a, double *b)
{
	if(a[0] >= b[3] || a[1] >= b[4] || a[2] >= b[5] ||
	        a[3] <= b[0] || a[4] <= b[1] || a[5] <= b[2]
	  )
		return 1;

	return 0;
}


int bb_empty(double *a)
{
	if(a[0] == a[3] || a[1] == a[4] || a[2] == a[5])
		return 1;

	return 0;
}



//expand bounding boxes to exactly match resolution grid
void bb_match_resolution_expand(struct BBLIST **head, double resolution)
{
	if(!(*head))
		return;

	struct BBLIST *bl = *head;

	do
		{
			double r;
			double *box = bl->bb;
			
			bb_normalize(box);
			
			r = fmod(box[0], resolution);
			if(r!=0.0)			
				box[0] -= resolution-r; 
				
			r = fmod(box[1], resolution);
			if(r!=0.0)			
				box[1] -= resolution-r; 
			
			r = fmod(box[2], resolution);
			if(r!=0.0)			
				box[2] -= resolution-r; 
			
			
			
			r = fmod(box[3], resolution);
			if(r!=0.0)			
				box[3] += resolution-r; 
				
			r = fmod(box[4], resolution);
			if(r!=0.0)			
				box[4] += resolution-r; 
			
			r = fmod(box[5], resolution);
			if(r!=0.0)			
				box[5] += resolution-r; 
		}
	while((bl = bl->next) != NULL);
}




void bb_list_add(struct BBLIST **head, double *bb)
{
	struct BBLIST *bl;
	bl = malloc(sizeof(struct BBLIST));
	memcpy(bl->bb, bb, sizeof(double) * 6);
	bl->next = NULL;
	

	if(!(*head))
		{
			head[0] = bl;
			return;
		}

	struct BBLIST *c = *head;

	while(c->next)
		c = c->next;

	c->next = bl;
}

int bb_list_len(struct BBLIST *item)
{
	int r = 0;

	if(!item)
		return r;

	do
		{
			r++;
		}
	while( (item = item->next) );

	return r;
}

void bb_list_remove(struct BBLIST **head, struct BBLIST *item)
{
	if(!head[0])
		return;

	struct BBLIST *last = NULL;
	struct BBLIST *bl;
	bl = *head;

	if(item == bl)
		{
			head[0] = item->next;
			free(item);
			return;
		}

	last = bl;
	bl = bl->next;

	do
		{
			if(bl == item)
				{
					last->next = item->next;
					free(item);
					return;
				}

			last = bl;
		}
	while((bl = bl->next) != NULL);
}

void bb_list_free(struct BBLIST **head)
{
	if(!head[0])
		return;

	struct BBLIST *bl = *head;
	struct BBLIST *temp;

	do
		{
			temp = bl->next;
			free(bl);
		}
	while((bl = temp) != NULL);

	head[0] = NULL;
}


//expand a with b
void bb_expand_box(double *a, double *b)
{
	if(a[0] > b[0])
		a[0] = b[0];

	if(a[1] > b[1])
		a[1] = b[1];

	if(a[2] > b[2])
		a[2] = b[2];

	if(a[3] < b[3])
		a[3] = b[3];

	if(a[4] < b[4])
		a[4] = b[4];

	if(a[5] < b[5])
		a[5] = b[5];
}






inline double bb_volume(double *bb)
{
	return (bb[3] - bb[0]) * (bb[4] - bb[1]) * (bb[5] - bb[2]);
}


//find how much volume of a is within b
double bb_shared_volume(double *a, double *b)
{
	double t[6];
	memcpy(t, a, sizeof(double) * 6);

	//create intersection box
	if( t[0] < b[0] )
		t[0] = b[0];

	if(t[3] > b[3])
		t[3] = b[3];

	if(t[3] <= t[0])
		return 0.0;

	if( t[1] < b[1] )
		t[1] = b[1];

	if(t[4] > b[4])
		t[4] = b[4];

	if(t[4] <= t[1])
		return 0.0;

	if( t[2] < b[2] )
		t[2] = b[2];

	if(t[5] > b[5])
		t[5] = b[5];

	if(t[5] <= t[2])
		return 0.0;

	return bb_volume(t);
}


/*find extents of box list*/
void bb_list_extents(struct BBLIST **head, double *bbox)
{
	struct BBLIST *cursor;
	
	bbox[0]=0.0;
	bbox[1]=0.0;
	bbox[2]=0.0;
	bbox[3]=0.0;
	bbox[4]=0.0;
	bbox[5]=0.0;
	
	
	cursor = *head;
	do 
	{	
		bb_expand(bbox, cursor->bb);	
	}while((cursor = cursor->next) != NULL);
		
}

/*
return 1 if y is inside a bbox in the specified dimension, otherwise 0
*/
int bb_list_dim_test(struct BBLIST **head, int dimension, double y)
{
	struct BBLIST *cursor;
	
	
	//is y currently in a bbox?
	cursor = *head;
	do 
	{	
		if(y>=cursor->bb[dimension] && y<cursor->bb[dimension+3]) //yes was inside this one
			return 1;		
	}while((cursor = cursor->next) != NULL);
	
	return 0;
}

/*
get next bbox start/stop in specified dimension, if finished return 0 else 1
must extend into other bboxes that might overlap but continue further
*/
int bb_list_next_lowest(struct BBLIST **head, int dim, double y, double *returned_start, double *returned_stop)
{
	struct BBLIST *cursor;
		
	double span_start=y;
	double span_end= -DBL_MAX;

	//find nearest starting point
	int inside=0;
	double minds = DBL_MAX;
	double minds_end = -DBL_MAX;
	
	cursor = *head;		
	do 
	{	
		double ds =  cursor->bb[dim];
		double de =  cursor->bb[dim+3];
		
		if(y>=ds && y<de) //currently inside a segment
		{
			inside = 1;
			span_end=de;
			break;
		}
		
		if(y<ds && ds<minds)
		{
			minds=ds;
			minds_end = de;
		}
		
	}while((cursor = cursor->next) != NULL);
	
	if((!inside) && minds!=DBL_MAX)
	{
		span_start = minds;
		span_end = minds_end;
	}
	
			
	//how far can the end point be extended?
	int extended = 1;
	while(extended)
	{
		extended=0;
		cursor = *head;		
		do 
		{	
			double ds =  cursor->bb[dim];
			double de =  cursor->bb[dim+3];
			
			if(span_end>=ds && span_end<de)
			{			
				span_end=de;
				extended++;
			}			
		}while((cursor = cursor->next) != NULL);
	}
	
	if(span_end==-DBL_MAX)
		return 0;
	
	returned_start[0]=span_start;
	returned_stop[0]=span_end;
	
	return 1;	
}




