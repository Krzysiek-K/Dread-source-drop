NF==3 {
    frame = $1;

    prefix = frame;
    sub(/_.*$/,"",prefix);

    corename = frame;
    sub(/^[^_]*_/,"",corename);

    if( prefix=="SP" )
    {
        OFFX[corename] = $2 + 0;
        OFFY[corename] = $3 + 0;
    }
    else
    {
        printf("\tframe\t-\tSP_%s\t%4d %4d\n",corename,OFFX[corename]-$2,OFFY[corename]-$3+6);
    }
}