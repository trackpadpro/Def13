% A = [0 1;1 0];
% eig(A)
% 
% A = [7 16 11 -16;2 13 -8 2;1 16 18 -5;-13 -14 -9 -2];
% eig(A)

[y,Fs] = audioread("K:\devSandbox\Def13\data\audio\wastemytime.mp3");
y = y(:,1); %swap to only one channel
n = length(y);
F = fft(y(:,1));
Fcenter = abs(fftshift(F));

% sound(y,Fs)

cla
subplot(1,2,1)
plot(Fs/n*(-n/2:n/2-1),Fcenter,"LineWidth",1)
xlabel("f (Hz)")
hold on

subplot(1,2,2)
spectrogram(y)

% %narrow frequencies of interest - not worth running
% for i = n:-1:1
%     if Fcenter(i,1)<5
%         Fcenter(i,:) = [];
%     end
% end
% 
% N = length(Fcenter(:,1));
% subplot(1,2,2)
% plot(Fs/n*(-N/2:N/2-1),Fcenter,"LineWidth",2)
% xlabel("f (Hz)")
