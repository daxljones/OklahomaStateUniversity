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
    A = ones(length(x), m+1); % make placeholder matrix
    
    for i = m:-1:1 %decrement from m through matrix cols
       A(:, i) = A(:, i+1).*x; %calculate power of each col
       
    end

    p = (A \ y'); %calculate coefficients
       
end