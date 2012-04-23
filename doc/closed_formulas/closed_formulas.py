# This is a script to test closed formulas for the following
# recurrent expressions:
# multiplication: rec_mul and closed_mul
#     a_0 = alpha
#     a_n = beta * a_{n-1} + gamma
#  
# and division: rec_div and closed_div
#     a_0 = alpha
#     a_n = 1/beta * a_{n-1} + gamma

__author__ = "Artem Shinkarov"
__date__ = "01/09/2011"

# Recursive division variant
def rec_div (n, alpha, beta, gamma):
  if n == 0:
    return alpha

  return 1./beta * rec_div (n-1, alpha, beta, gamma) + gamma

# Closed division variant
def closed_div (n, alpha, beta, gamma):
  return gamma * beta / (beta - 1.) \
	 + (alpha * beta - gamma * beta - alpha) /\
	   (beta - 1.) * (1. / beta ** n)

# Recursive multiplication variant
def rec_mul (n, alpha, beta, gamma):
  if n == 0:
    return alpha

  return beta * rec_mul (n-1, alpha, beta, gamma) + gamma

# Closed multiplication variant
def closed_mul (n, alpha, beta, gamma):
  return gamma / (1. - beta) \
	 + (alpha - gamma - beta * alpha) / (1. - beta) \
	   * beta ** n


if __name__ == "__main__":
  alpha = 1435.
  beta = 2.
  gamma = -12.

  for i in xrange (10):
    r = rec_div (i, alpha, beta, gamma)
    c = closed_div (i, alpha, beta, gamma)
    assert (c == r)
    print "%f  =?= %f %s" % (c, r, repr (c == r))

  for i in xrange (10):
    r = rec_mul (i, alpha, beta, gamma)
    c = closed_mul (i, alpha, beta, gamma)

    assert (c == r)
    print "%f  =?= %f %s" % (c, r, repr (c == r))
