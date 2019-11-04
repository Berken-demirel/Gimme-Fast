 f1 = 2;  % cuttoff low frequency to get rid of baseline wander                                                                 
 f2 = 30; % cuttoff frequency to discard high frequency noise
 Fs = 329; % Sampling Rate
 Wn = [f1 f2] * 2 / Fs ;                    
 N = 3;     % order of 3 less processing                                                               
 [b, a] = butter(N,Wn,'bandpass'); %Butterworth filter design 
 
