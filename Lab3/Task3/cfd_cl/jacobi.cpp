#include <stdio.h>

#include "jacobi.h"


float deltasq(float *newarr, float *oldarr, int m, int n)
{
	int i, j;

	float dsq=0.0;
	float tmp;

	for(i=1;i<=m;i++)
	{
		for(j=1;j<=n;j++)
	{
		tmp = newarr[i*(m+2)+j]-oldarr[i*(m+2)+j];
		dsq += tmp*tmp;
		}
	}

	return dsq;
}
