function [pass, d_path] = spotter_06(out_path)
% OUT_PATH -> Path to where eventually error files will be written
% PASS     -> Logical variable with the error status (PASS == true means the test passed)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	d_path = [pato filesep];

	ans1 = '157.32/-80.44/11.97';
	ans2 = '162.38/-72.57/11.62';
	res1 = gmt('rotconverter 150.1/70.5/-20.3 + 145/40/11.4 --FORMAT_GEO_OUT=D --FORMAT_FLOAT_OUT=%.2f --IO_COL_SEPARATOR=/');
	res1 = sprintf('%.2f/%.2f/%.2f', res1.data(1:3));	% It does not return a string like the cmd line version
	res2 = gmt('rotconverter 351.4/80.8/-22.5 + 62.26/85.36/11.14 --FORMAT_GEO_OUT=D --FORMAT_FLOAT_OUT=%.2f --IO_COL_SEPARATOR=/');
	res2 = sprintf('%.2f/%.2f/%.2f', res2.data(1:3));
	pass = isequal(res1, ans1);
	if (~pass)
		fid = fopen([out_path fname '_fail1.dat'], 'w');
		fprintf(fid, 'Should be\n%s\nIt is\n%s', num2str(ans1), num2str(res1.data));
		fclose(fid);
	end
	pass2 = isequal(res2, ans2);
	if (~pass2)
		fid = fopen([out_path fname '_fail2.dat'], 'w');
		fprintf(fid, 'Should be\n%s\nIt is\n%s', num2str(ans2), num2str(res2));
		fclose(fid);
	end
	pass = pass && pass2;
