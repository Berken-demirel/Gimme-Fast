class SharpSens
{
  public:
    SharpSens(const int _in1,const int _in2,const int _len);
    double getRelativePos();
    double getRightDistance();
    double getLeftDistance();
  private:
    int in1,in2,len;
    uint16_t *_raw1,*_raw2; 
    const double coeff[6]={-3.29046402122763e-15, 2.91509592719523e-11,  -9.76838924094621e-08, 0.000156303914505686,-0.127290461796245,55.8217296080085};
};

SharpSens::SharpSens(const int _in1,const int _in2,const int _len)
{
  in1=_in1;
  in2=_in2;
  len=_len;
  _raw1=new uint16_t[_len];
  _raw2=new uint16_t[_len];
  for(int i=0;i<_len;i++)
  {
    _raw1[i]=0;
    _raw2[i]=0;
  }
}
double SharpSens::getRelativePos()
{
  return getRightDistance()-getLeftDistance();
}
double SharpSens::getRightDistance()
{ 
  double sum=0;
  for(int i=0;i<len-1;i++)
  {
    _raw1[i+1]=_raw1[i];
    if(i>0)
      sum+=_raw1[i];
  }
  _raw1[0]=analogRead(in2);
   sum=(sum+_raw1[0])/(double)len;
   sum=pow(sum,5) *(-3.29046402122763e-15)+pow(sum,4)*(2.91509592719523e-11)+pow(sum,3)*(-9.76838924094621e-08)+pow(sum,2)*(0.000156303914505686)+sum*(-0.127290461796245)+55.8217296080085;
   return sum;
}

double SharpSens::getLeftDistance()
{
  double sum=0;
  for(int i=0;i<len-1;i++)
  {
    _raw2[i+1]=_raw2[i];
    if(i>0)
      sum+=_raw2[i];
  }
  _raw2[0]=analogRead(in1);
  sum=(sum+_raw2[0])/(double)len;
  sum=pow(sum,5) *(-3.29046402122763e-15)+pow(sum,4)*(2.91509592719523e-11)+pow(sum,3)*(-9.76838924094621e-08)+pow(sum,2)*(0.000156303914505686)+sum*(-0.127290461796245)+55.8217296080085;
  return sum;
}
