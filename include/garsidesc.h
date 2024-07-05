#include "cgarside.h"
#include "garsidesss.h"

namespace SC
{
  using namespace CGarside;

  template <class F>
  std::vector<Braid<F>> Trajectory(Braid<F> b)
  {
    std::vector<Braid<F>> t;
    std::unordered_set<Braid<F>> t_set;

    while (t_set.find(b) == t_set.end())
    {
      t.push_back(b);
      t_set.insert(b);
      b.Sliding();
    }

    return t;
  }

  template <class F>
  std::vector<Braid<F>> Trajectory(Braid<F> b, Braid<F> &c, sint16 &d)
  {
    std::vector<Braid<F>> t;
    std::unordered_set<Braid<F>> t_set;

    c.Identity();
    d = 0;

    while (t_set.find(b) == t_set.end())
    {
      t.push_back(b);
      t_set.insert(b);
      c.RightProduct(b.PreferredPrefix());
      b.Sliding();
      d++;
    }

    Braid<F> b2 = b;
    Braid<F> c2 = Braid(b2.PreferredPrefix());
    b2.Sliding();
    d--;
    while (b2 != b)
    {
      c2.RightProduct(PreferredPrefix(b2));
      b2.Sliding();
      d--;
    }
    c.RightDivide(c2);

    return t;
  }

  template <class F>
  Braid<F> SendToSC(const Braid<F> &b)
  {
    Braid<F> b_sc = Trajectory(b).back();
    b_sc.Sliding();
    return b_sc;
  }

  template <class F>
  Braid<F> SendToSC(const Braid<F> &b, Braid<F> &c)
  {
    sint16 d;
    Braid<F> b_sc = Trajectory(b, c, d).back();
    b_sc.Sliding();
    return b_sc;
  }

  template <class F>
  F Transport(const Braid<F> &b, const F &f)
  {
    Braid<F> b2 = b;
    b2.Conjugate(f);
    Braid<F> b3 = !Braid(b.PreferredPrefix()) * f * b2.PreferredPrefix();

    F f2 = F(b.GetParameter());

    if (b3.CanonicalLength() > 0)
    {
      f2 = b3.FactorList.front();
    }
    else if (b3.Delta == 1)
    {
      f2.Delta();
    }
    else
    {
      f2.Identity();
    }

    return f2;
  }

  template <class F>
  std::list<F> Return(const Braid<F> &b, const F &f)
  {
    std::list<F> ret;
    std::unordered_set<F> ret_set;
    F g = f;

    Braid<F> b1 = b;
    sint16 i, N = 1;

    b1.Sliding();

    while (b1 != b)
    {
      N++;
      b1.Sliding();
    }

    while (ret_set.find(g) == ret_set.end())
    {
      ret.push_back(g);
      ret_set.insert(g);

      b1 = b;
      for (i = 0; i < N; i++)
      {
        g = Transport(b1, g);
        b1.Sliding();
      }
    }

    while (ret.front() != g)
    {
      ret.pop_front();
    }

    return ret;
  }

  template <class F>
  F Pullback(const Braid<F> &b, const F &f)
  {
    Braid<F> b2 = Braid(b.PreferredPrefix());
    b2.RightProduct(f);
    Braid<F> b3 = b;
    b3.Sliding();
    b3.Conjugate(f);
    F f2 = b3.PreferredSuffix();

    Braid<F> c = b2.RightMeet(f2);

    b2.RightDivide(c);

    if (b2.IsIdentity())
    {
      f2.Identity();
      return f2;
    }
    else if (b2.CanonicalLength() == 0)
    {
      f2.Delta();
      return f2;
    }
    else
    {
      return b2.FactorList.front();
    }
  }

  template <class F>
  F MainPullback(const Braid<F> &b, const F &f)
  {
    std::vector<F> ret;
    std::unordered_set<F> ret_set;

    Braid<F> b2 = b;

    std::vector<Braid<F>> t = Trajectory(b);

    if (f.IsDelta())
    {
      return f;
    }

    F f2 = f;
    while (ret_set.find(f2) == ret_set.end())
    {
      ret.push_back(f2);
      ret_set.insert(f2);

      for (typename std::vector<Braid<F>>::reverse_iterator itb = t.rbegin();
           itb != t.rend();
           itb++)
      {
        f2 = Pullback(*itb, f2);
      }
    }
    return f2;
  }

  template <class F>
  F MinSC(const Braid<F> &b, const Braid<F> &b_rcf, const F &f)
  {
    F f2 = SSS::MinSSS(b, b_rcf, f);

    std::list<F> ret = Return(b, f2);
    for (typename std::list<F>::iterator it = ret.begin(); it != ret.end(); it++)
    {
      if ((f ^ *it) == f)
      {
        return *it;
      }
    }

    f2 = MainPullback(b, f);

    ret = Return(b, f2);

    for (typename std::list<F>::iterator it = ret.begin(); it != ret.end(); it++)
    {
      if ((f ^ *it) == f)
      {
        return *it;
      }
    }

    f2.Delta();

    return f2;
  }

  template <class F>
  std::vector<F> MinSC(const Braid<F> &b)
  {
    F f = F(b.GetParameter());
    Braid<F> b_rcf = b;
    b_rcf.MakeRCFFromLCF();
    std::vector<F> atoms = f.Atoms();
    std::vector<F> factors = f.Atoms();

    std::transform(std::execution::par,
                   atoms.begin(),
                   atoms.end(),
                   factors.begin(),
                   [&b, &b_rcf](F &atom) {return MinSC(b, b_rcf, atom);});

    std::vector<F> min;

    bool table[atoms.size()] = {false};
    bool should_be_added;

    for (sint16 i = 0; i < int(atoms.size()); i++)
    {
      f = factors[i];
      should_be_added = true;

      // We check, before adding f, that a divisor of it wasn't added already with some other atom dividing it.
      for (sint16 j = 0; j < i && should_be_added; j++)
      {
        should_be_added = !(table[j] && ((atoms[j] ^ f) == atoms[j]));
      }
      // If that is not the case, we also check if the atom we see is the last that might generate f.
      // This is to avoid duplicates; furthermore, if some strict left divisor of f can be generated by an atom,
      // doing things in this order ensures that it would have been detected by the first loop by the time we try to add f.
      for (sint16 j = i + 1; (j < int(atoms.size())) && should_be_added; j++)
      {
        should_be_added = !((atoms[j] ^ f) == atoms[j]);
      }
      if (should_be_added)
      {
        min.push_back(f);
        table[i] = true;
      }
    }

    return min;
  }

  // A SCS is stored as both an union of (disjoint) orbits, and a set.
  // The set is actually a map: for each key it stores the orbit it belongs to (as an index referring to Orbits).
  // Both are built concurrently; the set part is used to speed up membership tests.
  // Up to names, this is exactly the same data structure as `USS::UltraSummitSet`.
  template <class B>
  class SlidingCircuitSet
  {
  private:
    std::vector<std::vector<B>> Orbits;
    std::unordered_map<B, sint16> Set;

  public:
    // Adds a trajectory to the SCS.
    // Linear in the trajectory's length.
    inline void Insert(std::vector<B> t)
    {
      Orbits.push_back(t);
      for (typename std::vector<B>::iterator it = t.begin(); it != t.end(); it++)
      {
        Set.insert(std::pair(*it, int(Orbits.size()) - 1));
      }
    }

    // Checks membership.
    inline bool Mem(const B &b) const
    {
      return Set.find(b) != Set.end();
    }

    // Finds b's orbit.
    inline sint16 Orbit(const B &b) const
    {
      return Set.at(b);
    }

    inline sint16 NumberOfOrbits() const
    {
      return Orbits.size();
    }

    inline sint16 Card() const
    {
      return Set.size();
    }

    void Print(std::ostream &os) const
    {
      for (sint16 i = 0; i < int(Orbits.size()); i++)
      {
        os << "Orbit " << i << ":" << std::endl;
        for (sint16 j = 0; j < int(Orbits[i].size()); j++)
        {
          Orbits[i][j].Print(os);
          os << std::endl;
        }
      }
    }

    void Debug(std::ostream &os) const
    {
      for (sint16 i = 0; i < int(Orbits.size()); i++)
      {
        os << "Orbit " << i << ":" << std::endl;
        for (sint16 j = 0; j < int(Orbits[i].size()); j++)
        {
          Orbits[i][j].Debug(os);
          os << std::endl;
        }
      }
    }
  };

  template <class F>
  SlidingCircuitSet<Braid<F>> SCS(Braid<F> b)
  {
    SlidingCircuitSet<Braid<F>> scs;
    std::list<Braid<F>> queue;
    F f = F(b.GetParameter());

    Braid<F> b2 = SendToSC(b);
    scs.Insert(Trajectory(b2));
    queue.push_back(b2);

    F delta = F(b.GetParameter());
    delta.Delta();

    b2.Conjugate(delta);

    if (!scs.Mem(b2))
    {
      scs.Insert(Trajectory(b2));
      queue.push_back(b2);
    }

    while (!queue.empty())
    {
      std::vector<F> min = MinSC(queue.front());

      for (typename std::vector<F>::iterator itf = min.begin(); itf != min.end(); itf++)
      {
        b2 = queue.front();
        b2.Conjugate(*itf);

        if (!scs.Mem(b2))
        {
          scs.Insert(Trajectory(b2));
          queue.push_back(b2);
          b2.Conjugate(delta);
          if (!scs.Mem(b2))
          {
            scs.Insert(Trajectory(b2));
            queue.push_back(b2);
          }
        }
      }
      queue.pop_front();
    }
    return scs;
  }

  template <class F>
  SlidingCircuitSet<Braid<F>> SCS(const Braid<F> &b, std::vector<F> &mins, std::vector<sint16> &prev)
  {
    SlidingCircuitSet<Braid<F>> scs;
    std::list<Braid<F>> queue;
    F f = F(b.GetParameter());

    sint16 current = 0;
    mins.clear();
    prev.clear();

    Braid<F> b2 = SendToSC(b);

    scs.Insert(Trajectory(b2));
    queue.push_back(b2);

    while (!queue.empty())
    {
      std::vector<F> min = MinSC(queue.front());

      for (typename std::vector<F>::iterator itf = min.begin(); itf != min.end(); itf++)
      {
        b2 = queue.front();
        b2.Conjugate(*itf);

        if (!scs.Mem(b2))
        {
          scs.Insert(Trajectory(b2));
          queue.push_back(b2);
          mins.push_back(*itf);
          prev.push_back(current);
        }
      }
      queue.pop_front();
      current++;
    }
    return scs;
  }

  template <class F>
  Braid<F> TreePath(const Braid<F> &b, const SlidingCircuitSet<Braid<F>> &scs, const std::vector<F> &mins, const std::vector<sint16> &prev)
  {
    Braid<F> c = Braid(b.GetParameter());

    if (b.CanonicalLength() == 0)
    {
      return c;
    }

    sint16 current = scs.Orbit(b);

    for (typename std::list<Braid<F>>::iterator itb = scs.Orbits[current].begin(); *itb != b; itb++)
    {
      c.RightProduct((*itb).PreferredPrefix());
    }

    while (current != 0)
    {
      c.LeftMultiply(mins[current]);
      current = prev[current];
    }

    return c;
  }

  template <class F>
  bool AreConjugate(const Braid<F> &b1, const Braid<F> &b2, Braid<F> &c)
  {
    sint16 n = b1.GetParameter();
    Braid<F> c1 = Braid<F>(n), c2 = Braid<F>(n);

    Braid<F> bt1 = SendToSC(b1, c1), bt2 = SendToSC(b2, c2);

    if (bt1.CanonicalLength() != bt2.CanonicalLength() || bt1.Sup() != bt2.Sup())
    {
      return false;
    }

    if (bt1.CanonicalLength() == 0)
    {
      c = c1 * !c2;
      return true;
    }

    std::vector<F> mins;
    std::vector<sint16> prev;

    SlidingCircuitSet<Braid<F>> scs = SCS(bt1, mins, prev);

    if (!scs.Mem(bt2))
    {
      return false;
    }

    c = c1 * TreePath(bt2, scs, mins, prev) * !c2;

    return true;
  }
}