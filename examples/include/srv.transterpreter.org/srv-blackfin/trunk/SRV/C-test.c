main()
{
    int i, x, y, z;
 
    color(0, 20, 250, 40, 200, 40, 200);
    imgrcap();
    imgdiff(1);
    delay(100);
    laser(1);
    delay(200);
    imgcap();
    delay(100);
    laser(0);
    i = blob(0, 0);
    print(i);
    imgdiff(0);
}

