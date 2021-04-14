function p = nonlinear(x, y, m)
%Use: find poly coefficients for line of best fit
%input:
%   x = series of x points
%   y = series of y points
%   m = desired order of coefficients
%output:
%   p = vector of coefficients

    if length(y) ~= length(x), error('x and y must be the same size'); end
    
    x = x(:); %transpose x
    V = ones(length(x), m+1); % make placeholder matrix
    
    for i = m:-1:1 %decrement from m through matrix cols
       V(:, i) = V(:, i+1).*x; %calculate power of each col
       
    end

    p = (V \ y'); %calculate coefficients
       
end