
% read images, convert to dat format, these are used in
% brahms_test binsource

im_j = imread('johannes-128.png');
im_b = imread('brahms-128.png');

% make johannes attractively sepia
sepia = [1 0.7 0.4];
for d = 1:3
	im_j(:,:,d) = round(im_j(:,:,d) * sepia(d));
end

% make notes a little green because it's my favourite colour
im_b(:,:,2) = double(im_b(:,:,2)) / 2 + 128;

im_x = complex(im_j, im_b);

% % for comparison
% im = im_x;
% save('complex-128', 'im');



% store
filenames = {'johannes-128' 'brahms-128' 'adjacent-128' 'interleaved-128'};
images = {im_j im_b im_x im_x};
for i = 1:4
	f = filenames{i};
	i = images{i};
	i = permute(i, [1 3 2]);
	fid = fopen([f '.dat'], 'wb');
	if isreal(i)
		fwrite(fid, i, '*uint8');
	else
		if strcmp(f, 'adjacent-128')
			fwrite(fid, real(i), '*uint8');
			fwrite(fid, imag(i), '*uint8');
		else
			for c = 1:128
				fwrite(fid, real(i(:,:,c)), '*uint8');
				fwrite(fid, imag(i(:,:,c)), '*uint8');
			end
		end
	end
	fclose(fid);
end

