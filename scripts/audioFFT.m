function [F] = audioFFT(x)
    if(isa(x,'string'))
        
    end

    n = length(x);
    f = 48000; %48kHz Discord audio
    F = abs(fftshift(fft(x)));
    subplot(1,2,1)
    plot(x)
    subplot(1,2,2)
    plot(f/n*(-n/2:n/2-1),F,"LineWidth",1)
    %sound(x,f)
end
