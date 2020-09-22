function varargout = gmt(cmd, varargin)
% Helper function to call the gmtmex MEX function

	if (nargin == 0)
		fprintf(sprintf('\n\t\tGMT - The Generic Mapping Tools, Version 6.1 API\n'))
		fprintf(sprintf('Copyright 1991-2020 The GMT Team (https://www.generic-mapping-tools.org/team.html\n\n'))
	
		fprintf(sprintf('Usage:\tTo call a GMT module:\n\t    output = gmt (''module_name'', ''options'', numeric_input)\n\n'))
		fprintf(sprintf(['\tTo create a Grid structure from a 2-D Z array and a 1x9 header vector:\n\t' ...
		                 '    G = gmt (''wrapgrid'', Z, head)\n' ...
				 '\theader is a vector with [x_min x_max, y_min y_max z_min z_max reg x_inc y_inc]\n\n']))
		fprintf(sprintf(['\tTo create an Image structure from a 2-D img array and a 1x9 header vector:\n\t' ...
		                 '    I = gmt (''wrapimage'', img, header [, cmap])\n' ...
						 '\theader is a vector with [x_min x_max, y_min y_max z_min z_max reg x_inc y_inc].\n' ...
						 '\tcmap is an optional color palette structure or a Matlab Mx3 cmap array (not yet).\n\n']))
		fprintf(sprintf(['\tTo create a structure for a multi-segment dataset:\n\t' ...
		                 '    D = gmt (''wrapseg'', {[1 0; 1 1], [5 5; 56]}, {''Seg1'', ''Seg2''})\n\n']))
		fprintf(sprintf(['\tTo create a structure with numeric and text good for use in pstext:\n\t' ...
		                 '    R = gmt (''record'', [1 0; 1 1], {''Text1'', ''Text2''})\n\n']))
		fprintf(sprintf(['\tTo join two color palette structures:\n\t' ...
		                 '    cpt = gmt (''catcpt'', cpt1, cpt2)\n\n']))
		fprintf(sprintf(['\tTo merge all data segments from an array of Data structures:\n\t' ...
		                 '    all = gmt (''catseg'', segments[, 1])\n\t' ...
		                        'The optional 2nd argument will insert a NaN-record at the start of each segment.\n']))
		return
	end

	if (strcmp(cmd,'wrapgrid') || strcmp(cmd,'wrapimage') || strcmp(cmd,'catcpt') || strncmp(cmd,'catseg',6) || ...
			strcmp(cmd,'wrapseg') || strcmp(cmd,'record'))
		[varargout{1:nargout}] = feval (cmd, varargin{:});
	else
		[varargout{1:nargout}] = gmtmex (cmd, varargin{:});
	end

% -------------------------------------------------------------------------------------------------
function all = catseg(varargin)
	all = catsegment(varargin{:});

function all = catsegment(A, header)
% MERGE  Combine all segment arrays to a single array
%   all = catsegment (A, opt)
%
% Concatenate all data segment arrays in the structures A
% into a single array.  If the optional argument opt is given
% the we start each segment with a NaN record.

	n_segments = length(A); n = 0;
	[nr, nc] = size (A(1).data);	% Get # columns from first segment
	for k = 1:n_segments		% Count total rows
	    n = n + length(A(k).data);
	end
	if nargin == 2 % Need to add a NaN-record per segment
	    all = zeros (n+n_segments, nc);
	else
	    all = zeros (n, nc);
	end
	n = 1;
	for k = 1:n_segments
	    [nr, nc] = size (A(k).data);
	    if nargin == 2 % Add NaN-record
	        all(n,:) = NaN;
	        n = n + 1;
	    end
	    all(n:(n+nr-1),:) = A(k).data;
	    n = n + nr;
	end

% -------------------------------------------------------------------------------------------------
function cpt = catcpt(cpt1, cpt2)
% Join two CPT1 and CPT2 color palette structures. 
% Note, the two palettes must be continuous across its common border. No testing on that is done here.
% NOT COMPLETE. NEEDS THE CPT, MODEL & COMMENT FIELDS

	if (nargin ~= 2)
		error('    Must provide 2 input arguments.')
	elseif (cpt1.depth ~= cpt2.depth)
		error('    Cannot join two palettes that have different bit depths.')
	end
	if (size(cpt1.colormap,1) ~= size(cpt1.range))
		% A continuous palette so the join would have one color in excess. We could average
		% the top cpt1 color and bottom cpt2 but that would blur the transition. 
		%cpt.colormap = [cpt1.colormap(1:end-1,:); (cpt1.colormap(end,:)+cpt2.colormap(1,:))/2; cpt2.colormap(2:end,:)];
		cpt.colormap = [cpt1.colormap(1:end-1,:); cpt2.colormap];
		cpt.alpha    = [cpt1.alpha(1:end-1,:);    cpt2.alpha];
	else
		cpt.colormap = [cpt1.colormap; cpt2.colormap];
		cpt.alpha    = [cpt1.alpha;    cpt2.alpha];
	end
	cpt.range  = [cpt1.range;    cpt2.range];
	cpt.minmax = [cpt1.minmax(1) cpt2.minmax(2)];
	cpt.bfn    = cpt1.bfn;			% Just keep the first one
	cpt.depth  = cpt1.depth;

% -------------------------------------------------------------------------------------------------
function G = wrapgrid(Z, head)
% Fill the Grid struct used in gmtmex. HEAD is the old 1x9 header vector.

	if (nargin ~= 2)
		error('    Must provide 2 input arguments.')
	elseif (size(Z,1) < 2 || size(Z,2) < 2)
		error('    First argument must be a decent 2D array.')
	elseif (any(size(head) ~= [1 9]))
		error('    Second argument must be a 1x9 header vector.')
	end

	if (~isa(head, 'double')),	head = double(head);	end
	G.proj4 = '';
	G.wkt = '';	
	G.range = head(1:6);
	G.inc = head(8:9);
	G.registration = head(7);
	G.nodata = NaN;
	G.title = '';
	G.comment = '';
	G.command = '';
	G.datatype = 'float32';
	G.x = linspace(head(1), head(2), size(Z,2));
	G.y = linspace(head(3), head(4), size(Z,1));
	G.z = Z;
	G.x_unit = '';
	G.y_unit = '';
	G.z_unit = '';	

% -------------------------------------------------------------------------------------------------
function I = wrapimage(img, head, cmap)
% Fill the Image struct used in gmtmex. HEAD is the old 1x9 header vector.

	if (nargin < 2)
		error('    Must provide at least 2 input arguments.')
	end
	if (size(img,1) < 2 || size(img,2) < 2)
		error('    First argument must be a decent 2D image array.')
	elseif (any(size(head) ~= [1 9]))
		error('    Second argument must be a 1x9 header vector.')
	end

	if (~isa(head, 'double')),	head = double(head);	end
	I.proj4 = '';
	I.wkt = '';	
	I.range = head(1:6);
	I.inc = head(8:9);
	I.nodata = NaN;
	I.registration = head(7);
	I.title = '';
	I.comment = '';
	I.command = '';
	I.datatype = 'uint8';
	I.x = linspace(head(1), head(2), size(img,2));
	I.y = linspace(head(3), head(4), size(img,1));
	I.image = img;
	I.x_unit = '';
	I.y_unit = '';
	I.z_unit = '';	
	if (nargin == 3)
		if (~isa(cmap, 'struct'))
			% TODO: write a function that converts from Mx3 Matlab cmap to color struct used in MEX
			error('The third argin (cmap) must be a colormap struct.')
		end
		I.colormap = cmap;	
	else
		I.colormap = [];	
	end
	if (size(img,3) == 4)			% Not obvious that this is the best choice
		I.alpha = img(:,:,4);
	else
		I.alpha = [];	
	end
	I.layout = 'TCBa';

% -------------------------------------------------------------------------------------------------
function D = wrapseg(in, headers, text, comm, proj_s, wkt_s)
% Fill the Dataset struct used in gmtmex.
% IN -> A cell array of matrices where each matrix is a segment.
% HEADERS, TEXT, COMM, PROJ_S & WKT_S are all optional and can either be empty, char strings or cell
% arrays of text with the exact same size of input data IN which must also be a cell array (numeric).
% HEADERS  -> multi-segment header info (a cell array of strings o a single text line)
% TEXT     -> any text that may follow numeric columns (a cell array of strings or a single text line)
% COMM     -> Commentary describing this datase (a text string)
% PROJ_S   -> A text string with a PROJ4 projection string
% WKT_S    -> A text string with a WKT projection string

	if (nargin == 0),       error('Wrong number of input arguments'),	end
	if (~isa(in, 'cell')),  error('Only cell arrays of matrices are accepted in first argument'),	end
	if (~exist('headers', 'var') || isempty(headers)),  headers = cell(size(in));    end
	if (~exist('text', 'var')    || isempty(text)),     text = cell(size(in));       end
	if (~exist('comm', 'var')    || isempty(comm)),     comm = cell(size(in));       end
	if (~exist('proj_s', 'var')  || isempty(proj_s)),   proj_s = cell(size(in));     end
	if (~exist('wkt_s', 'var')   || isempty(wkt_s)),    wkt_s = cell(size(in));      end
	if (isa(comm, 'char'))		% If allocated above this test is false
		comment = cell(size(in));	comment{1} = comm;
	else
		comment = comm;
	end
	if (isa(proj_s, 'char'))
		proj4 = cell(size(in));		proj4{1} = proj_s;
	else
		proj4 = proj_s;
	end
	if (isa(wkt_s, 'char'))
		wkt = cell(size(in));		wkt{1} = wkt_s;
	else
		wkt = wkt_s;
	end
	if (~isequal(size(in), size(headers)) || ~isequal(size(headers), size(text)) || ~isequal(size(text), size(comm)) || ...
			~isequal(size(comm), size(proj_s)) || ~isequal(size(proj_s), size(wkt_s)))
		error('All cell arrays must be of the same size/shape. Can''t mix row and column cell vectors')
	end
	D = struct('data',in, 'header',headers, 'text',text, 'comment',comment, 'proj4',proj4, 'wkt',wkt);

% -------------------------------------------------------------------------------------------------
function R = record(data, text)
% Simplifies creating one or more GMT records on the fly
	R.data = data;
	if (ischar(text))
		R.text = text;
	else
		[n,m] = size(text);
		if (n == 1)
			R.text = text';
		else
			R.text = text;
		end
	end
