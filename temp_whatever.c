static int32 drawBoard(void *board, uint32 *lineLength)
{
  uint32 x;
  uint32 y;

  for(y=0; y<BOARDHEIGHT; y++)
  {
    //if nothing on this line goto the next line
    if(lineLength[y] == 0)
    {
      //nothing on line

    }
    else
    {
      //if somthing on line print upto the last game piece
      for(x=0; x<=lineLength[y]; x++)
      {
        //printf("%c",gameBoard[x][y] );
      }
    }   

    printf("x=%d, y=%d, lineLength[y] = %d \n", x,y, lineLength[y]);
  }
  
  //debug
  //printf("(%d=%d)",y, lastLinePosition[y] );

  //#ifdef WINV
  //goto next line, but not on last line else weirdness
  //  if (y<BOARDHEIGHT)
  //  {
  //    printf("\n");
  //  }
  //#else //if real terminal always need a new line!!
  //  printf("\n");
  //#endif

  return 0;
}
