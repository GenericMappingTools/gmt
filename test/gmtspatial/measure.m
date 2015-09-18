function [pass, d_path] = measure(out_path)
% OUT_PATH -> Path to where eventually error files will be written
% PASS     -> Logical variable with the error status (PASS == true means the test passed)
% PATH     -> Path to where this file lives (usefull for gmtest.m)
%
%	$Id$

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	d_path = [pato filesep];

	area = [0 0; 1 0; 1 1; 0 1; 0 0];
	% Cartesian centroid and area
	%echo "0.5	0.5	1" > answer
	answer = [0.5	0.5	1];
	result = gmt('gmtspatial -Q', area);
	pass = isequal(result, answer);
	if (~pass)
		fid = fopen([out_path fname '_fail1.dat'], 'w');
		fprintf(fid, 'Should be\n%s\nIt is\n%s', num2str(answer), num2str(result));
		fclose(fid);
	end
	% Geographic centroid and area
	answer = [0.5	0.500019038226	12308.3096995];
	result = gmt('gmtspatial -Q -fg', area);	
	pass = isequal(result, answer);
	if (~pass)
		fid = fopen([out_path fname '_fail2.dat'], 'w');
		fprintf(fid, 'Should be\n%s\nIt is\n%s', num2str(answer), num2str(result));
		fclose(fid);
	end
