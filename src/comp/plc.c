/*--------------------  file  --  PLC.C --------------------*/
/*     standard PL/I-functions implemented on C-language    */
/*                Last edition date : 10.02.86              */
/*----------------------------------------------------------*/
index_x (s,t)
          /* first left insertion t into  s */
          /* if not: -1                     */
           char *s, *t;
              {
               int l,m,imax;
               int i, j, k;
             imax = 40;
									                  for (i = 0; i < imax; i++) {
											      if (*(s+i) == '\0') break;
											      else l = i+1;
											   }/* l - length of s */

									     		  for (i = 0; i < imax; i++) {
								                            if (*(t+i) == '\0') break;
									                   else m = i+1;
									              }/* m - length of t */

										    for (i = 0; (i < l) && (*(s+i) != '\0') ; i++)
										    for ( j=i, k=0; (j < l) && (*(s+j) != '\0'); j++ ){
																	                         if (*(s+j) != *(t+k)) break;
																		                     k++;
																	                         if ( k >= m ) return(i);
																			                  }
									                  return(-1);
									              }


index(s,l,t,m)
   /* first left insertion t into  s */
   /* if not: -1                     */
 char *s, *t;
 int l,m;
 {
  int i, j, k;
  for (i = 0; (i < l) && (*(s+i) != '\0') ; i++)
    for ( j=i, k=0; (j < l) && (*(s+j) != '\0'); j++ ){
       if (*(s+j) != *(t+k)) break;
       k++;
       if ( k >= m ) return(i);
    }
  return(-1);
}
cmpstr(n,s1,s2) /* comparison two string . if s1<s2 then return < 0 */
              /* if s1 = s2 return 0. if s1>s2 then return > 0    */
 char *s1, *s2;
 int n;
 {
  int i;
  for (i=0; i<n; i++)
     if (*(s1+i) != *(s2+i)) return(*(s1+i) - *(s2+i));
  return(0);
 }
/*----------  end of file PLC.C  -----------*/
