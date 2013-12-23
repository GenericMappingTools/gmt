function split_file4coes(fname, n_int)
% Read the output of x2sys_cross -C<file> option and split it into N_INT files
%
% The rationale of this is that running x2sys_cross over a large number of files takes
% a lot of time and the problem is not easily parallelizable. An alternate solution is
% to split the main process into a series of sub-processes and launch each one at a
% different process of your multi-core machine. One could do a rough division by dividing
% by an equally number of lines of the FNAME file, but that is normally not good enough
% because run tine is proportional to the tracks cruises size and not on the number of them.
% An extra bonus is that we will also get rid of cruise pairs that are close enough to
% to have crossed, but actually do not cross. So, by knowing this (it's in the FNAME file),
% we don't need to waste time with such cases anymore.
%
% FNAME		Is the file name given to the -C option. It contains 4 columns.
%			First two hold the names of the pair of cruises whose COEs were under analysis
%			Third column has the number of cross-overs and Forth column the time taken
%			to compute all COEs of that pair. [Default 'runtimes.dat']
%
% N_INT		Is the number of sub-files that the FNAME file will be split [Default 8]
%			The sub-files are created in the same directory as FNAME and have name stem
%			of 'cruzados_%d.txt', e.g. cruzados_5.txt

%  -------------------------------------------------------------------------------------
% 	$Id$
% 
%       Copyright (c) 1999-2012 by J. Luis
%       See LICENSE.TXT file for copying and redistribution conditions.
%
%       This program is free software; you can redistribute it and/or modify
%       it under the terms of the GNU Lesser General Public License as published by
%       the Free Software Foundation; version 3 or any later version.
%
%       This program is distributed in the hope that it will be useful,
%       but WITHOUT ANY WARRANTY; without even the implied warranty of
%       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%       GNU Lesser General Public License for more details.
% 
%       Contact info: gmt.soest.hawaii.edu
%  -------------------------------------------------------------------------------------

	if (nargin == 0)
		fname = 'runtimes.dat';		n_int = 8;
	elseif (nargin == 1)
		n_int = 8;
	elseif (nargin ~= 2)
		error('Wrong number of input args')
	end

	PATO = fileparts(fname);
	fid = fopen(fname);
	todos = fread(fid,'*char');		fclose(fid);
	[fname1 fname2 n_cross times] = strread(todos,'%s %s %d %f');
	
	n_cross = logical(n_cross);
	fname1 = fname1(n_cross);			% Get rid of cruise pairs that never actually crossed each other
	fname2 = fname2(n_cross);
	times  = times(n_cross);

	acum_time = cumsum(times);			% Compute the acum_timeulated run-times

	dt = round(acum_time(end) / n_int);
	lim = [1 dt:dt:acum_time(end)+2];	% +2 to ensure that last chunk is not lost due to roundings
	lim(end) = acum_time(end);			% Reset the exact number of last element, which we know exactly
	ind = zeros(1,numel(lim));

	for (k = 2:numel(lim))
		difa = abs(acum_time - lim(k));
		[mimi, ind(k)] = min(difa);		% Find indices of where to split the main file
	end

	% Now write the N_INT splitted files
	for (k = 1:numel(lim)-1)
		fout = sprintf('cruzados_%d.txt', k);
		if (~isempty(PATO))				% Prepend path
			fout = [PATO filesep fout];
		end
		fid = fopen(fout, 'wt');
		for (n = ind(k)+1:ind(k+1))
			fprintf(fid, '%s\t%s\n', fname1{n}, fname2{n});		% Slower, but safer way
		end
		fclose(fid);
	end
