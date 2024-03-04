function [F] = audioFFT(x)
    n = length(x);
    f = 48000; %48kHz Discord audio
    F = abs(fftshift(fft(x)));
    plot(f/n*(-n/2:n/2-1),F,"LineWidth",1);
end
