class Lowpass:
    def __init__(self, inertia):
        self._last_val = None
        self._inertia = inertia

    def next(self, val):
        if self._last_val is None:
            self._last_val = val

        self._last_val = self._inertia * self._last_val + \
            (1. - self._inertia) * val
        return self._last_val


if __name__ == '__main__':
    data = [0., 0.1, 0.3, 0.2, 0.5, 0.4, 1., 1., 1.]
    lp = Lowpass(.5)
    for d in data:
        print(lp.next(d))
