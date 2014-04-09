% test source reading from binary file
sys = sml_system;

% get filename
sip = brahms_utils('GetSystemMLInstallPath');

wrong = false;

% process
state = [];
state.data = [sip '/BRAHMS/media/interleaved-128.dat'];
if wrong state.source = 'adjacent'; end
state.type = 'UINT8';
state.dims = [128 3 128];
state.repeat = true;
sys = sys.addprocess('src1', 'std/2009/source/numeric', 128, state);

% process
state = [];
state.data = [sip '/BRAHMS/media/adjacent-128.dat'];
state.sourceIsAdjacent = ~wrong;;
if wrong state.sourceIsAdjacent = false; end
state.type = 'UINT8';
state.dims = [128 3 128];
state.repeat = true;
sys = sys.addprocess('src2', 'std/2009/source/numeric', 128, state);

% execution
exe = brahms_execution;
exe.all = true;
exe.name = 'test';
exe.stop = 1;

% execute
[out, rep] = brahms(sys, exe);

% plot
figure(1)
clf
subplot(2,2,1)
im = permute(real(out.src1.out), [1 3 2]);
image(im)
title('interleaved (real)');
subplot(2,2,2)
im = permute(imag(out.src1.out), [1 3 2]);
image(im)
title('interleaved (imag)');
subplot(2,2,3)
im = permute(real(out.src2.out), [1 3 2]);
image(im)
title('adjacent (real)');
subplot(2,2,4)
im = permute(imag(out.src2.out), [1 3 2]);
image(im)
title('adjacent (imag)');
