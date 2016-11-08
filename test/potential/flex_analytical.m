function flex_analytical
% Script to calculate the Matlab exact solutions to disc flexure so that we
% can compare it to gravfft 3-D crossections.  The disc dimensions are
h = 4000; r = 15; rhol = 2800; rhoi = 2400; rhom = 3300; rhow = 1000;
Te = 10;
x = (0:512)';
% a) Regular single density case
wa = flexdisc (x, h, r, rhol, rhom, rhol, rhow, Te);
% b) Exact dual density case
wb = flexdisc (x, h, r, rhol, rhom, rhoi, rhow, Te);
% c) Approximate single density case
A = sqrt ((rhom - rhoi)/(rhom-rhol));
wc = A * flexdisc (x, h, r, rhoi, rhom, rhoi, rhow, Te);
figure(1);  clf;
plot (x, wb, 'g.', 'MarkerSize', 10); hold on
plot (x, wa, 'b-');
plot (x, wc, 'r-');
tmp = [x wa wb wc];
save flex_analytical.txt tmp -ascii -tabs

function w = flexdisc (x, h, r, rhol, rhom, rhoi, rhow, te)
% FLEXDISC calculates the flexural deformation caused by a
% disc of given dimensions. Infill density may differ from
% load density
%
% w = flexdisc (x, h, r, rhol, rhom, rhoi, rhow, te)
%
%     x		- where to evaluate flexure (all x >= 0) (km)
%     h		- height of disc (m)
%     r		- radius of disc (km)
%     rhol	- density of load
%     rhom	- density of mantle
%     rhoi	- density of infill
%     rhow	- density of water
%     te	- elastic plate thickness (km)
%
% On output, w is the deflections.

te = 1000.0 * te;
yi = flex3dk (te, rhom - rhol);		% Get inside flex wavenumber
yo = flex3dk (te, rhom - rhoi);		% Get outside flex wavenumber
airy = (rhol - rhow) / (rhom - rhol);	% Get density balance
in = find (x <= r);			% Get indices to points inside/outside
out = find (x > r);
xm = 1000.0 * x;			% Convert to meters
x(in) = xm(in) * yi;			% Scale to flexural wavelengths here instead of later
x(out) = xm(out) * yo;
ri = 1000.0 * r * yi;
if max(x(:)) > 8.0
	disp(['Warning: Solution inaccurate for distances beyond r = ' num2str(8/(1000.0*yo)) ' km'])
end
	% Evaluate the 4 constants

if (rhol == rhoi)	% Simple solution
	c = ri * dber (ri);
	d = -ri * dbei (ri);
	a = ri * dker (ri);
	b = -ri * dkei (ri);
else	% A bit more nasty
	ro = 1000.0 * r * yo;
	ber_ri = ber (ri);
	bei_ri = bei (ri);
	dber_ri = dber (ri);
	dbei_ri = dbei (ri);
	kei_ro = kei (ro);
	ker_ro = ker (ro);
	dkei_ro = dkei (ro);
	dker_ro = dker (ro);

	% Must solve for 4 coefficients by requiring continuity across disk end

	A = zeros (4,4);
	B = zeros (4,1);
	v1 = (1 - 0.25);

	% wi = w0

	A(1,1) = ber_ri;
	A(1,2) = bei_ri;
	A(1,3) = -ker_ro;
	A(1,4) = -kei_ro;
	B(1) = -1;

	% wi' = wo'

	A(2,1) = yi * dber_ri;
	A(2,2) = yi * dbei_ri;
	A(2,3) = -yo * dker_ro;
	A(2,4) = -yo * dkei_ro;

	% Mi = Mo

	A(3,1) = yi * yi * (bei_ri + v1 * dber_ri ./ ri);
	A(3,2) = -yi * yi * (ber_ri - v1 * dbei_ri ./ ri);
	A(3,3) = -yo * yo * (kei_ro + v1 * dker_ro ./ ro);
	A(3,4) = yo * yo * (ker_ro - v1 * dkei_ro ./ ro);

	% Qi = Qo

	A(4,1) = yi ^ 3 * dbei_ri;
	A(4,2) = -yi ^ 3 * dber_ri;
	A(4,3) = -yo ^ 3 * dkei_ro;
	A(4,4) = yo ^ 3 * dker_ro;

	X = A \ B;
	a = X(1);	b = X(2);	c = X(3);	d = X(4);
end

w = zeros (size (x));				% Get array for results

w(in) = -h * airy * (1.0 + a * ber (x(in)) + b * bei (x(in)));	% inside solution
w(out) = -h * airy * (c * ker (x(out)) + d * kei (x(out)));	% outside solution

function k = flex3dk (te, delrho)
% flex3dk calculates the flexural wavenumber for a
% 3-D plate on elastic foundation.
% k = flex3dk (te, delrho)
%     te	- the elastic thickness[es] in m
%     delrho	- density contrast in kg/m^3
%
%     the wavenumber is returned in 1/meters

	E = 7.0e10;
	v = 0.25;
	k = (((12.0 * delrho * 9.806199203 * (1.0 - v * v)) / E) ^ 0.25) * (te .^ (-0.75));
