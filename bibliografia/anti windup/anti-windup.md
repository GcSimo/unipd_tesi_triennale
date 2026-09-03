# Anti windup strategies

## Soluzioni

### Conditional integration
- resize the input reference to the controller to make the output of the controller feasible
- paper: "Integrator Windup and How to Avoid It", "Conditioning technique a general antiwindup and bumpless transfer method"

### Integral clamping
- disabling the integral function until the to-be-controlled process variable (PV) has entered the controllable region
Preventing the integral term from accumulating above or below pre-determined bounds

### Back-calculation
Back-calculating the integral term to constrain the process output within feasible bounds
proposed by Fertik and Ross in 1967
``output = k * (e + 1/t_i * integral(e - ti/(k*t_r) * (u - u_saturation) dt))``

### Clegg integrator
Zeroing the integral value every time the error is equal to, or crosses zero.


### Velocity algorithm
stop incrementing the output of the controller when the output is saturated, if the controller is implemented in velocity form (the controller compute the increment of the output, not the output itself)

### Integrator bounding
limiting the integral term to a certain range

### Observer-based anti-windup
...

### Linear matrix inequality (LMI) based anti-windup
...

## bibliografia visitata
Åström, K.J. and Hägglund, T. (2026). Introduction. In Advanced PID Control (eds K.J. Åström and T. Hägglund). https://doi.org/10.1002/9781394442102.ch1
@inbook{doi:https://doi.org/10.1002/9781394442102.ch1,
  publisher = {John Wiley & Sons, Ltd},
  isbn = {9781394442102},
  title = {Introduction},
  booktitle = {Advanced PID Control},
  chapter = {1},
  pages = {1-11},
  doi = {https://doi.org/10.1002/9781394442102.ch1},
  url = {https://onlinelibrary.wiley.com/doi/abs/10.1002/9781394442102.ch1},
  eprint = {https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781394442102.ch1},
  year = {2026},
  keywords = {adaptation, automatic tuning, commissioning, control loops, PID controller, set-point response, supervision},
  abstract = {Summary This introduction presents an overview of the key concepts discussed in the subsequent chapters of this book. The book provides the technical background for understanding proportional integral derivative (PID) control. It presents concepts that are useful for describing the behavior of processes. The book gives an in-depth presentation of the PID controller. This includes principles as well as many implementation details, such filtering to provide high-frequency roll-off,anti-windup,improvement of set-point response, etc. The book also gives a systematic design of feedforward control to improve set-point responses as well as a discussion of design of model-following systems. It describes methods for the design of PID controllers. The book discusses some techniques for adaptation and automatic tuning of PID controllers. It focuses on the methods for commissioning, supervision and diagnosis of control loops.}
}

K. J. Astrom and L. Rundqwist, "Integrator Windup and How to Avoid It," 1989 American Control Conference, Pittsburgh, PA, USA, 1989, pp. 1693-1698, doi: 10.23919/ACC.1989.4790464.
@INPROCEEDINGS{4790464,
  author={Astrom, Karl Johan and Rundqwist, Lars},
  booktitle={1989 American Control Conference},
  title={Integrator Windup and How to Avoid It},
  year={1989},
  volume={},
  number={},
  pages={1693-1698},
  abstract={This paper describes the phenomenon of integrator windup and various ways of avoiding it. It first covers a number of ad hoc schemes. A general procedure to avoid windup which admits a unification of the ideas is given and the results are illustrated on a number of examples.},
  keywords={Windup;Actuators;Control systems;Automatic control;Valves;Computer aided manufacturing;Nonlinear control systems;Feedback loop;Regulators;Three-term control},
  doi={10.23919/ACC.1989.4790464},
  ISSN={},
  month={June}
}

Sergio Galeani, Sophie Tarbouriech, Matthew Turner, Luca Zaccarian, "A Tutorial on Modern Anti-windup Design," European Journal of Control, Volume 15, Issues 3–4, 2009, Pages 418-440, ISSN 0947-3580, https://doi.org/10.3166/ejc.15.418-440.
@article{GALEANI2009418,
  title = {A Tutorial on Modern Anti-windup Design},
  journal = {European Journal of Control},
  volume = {15},
  number = {3},
  pages = {418-440},
  year = {2009},
  issn = {0947-3580},
  doi = {https://doi.org/10.3166/ejc.15.418-440},
  url = {https://www.sciencedirect.com/science/article/pii/S0947358009709987},
  author = {Sergio Galeani and Sophie Tarbouriech and Matthew Turner and Luca Zaccarian},
  keywords = {anti-windup, input saturation, saturated stability, saturated performance},
  abstract = {In this paper, several constructive linear and nonlinear anti-windup techniques are presented and explained. Two approaches, namely direct linear anti-windup (DLAW) and model recovery anti-windup (MRAW), are described in an algorithmic way, in order to illustrate their main features. Hereafter, theoretical conditions ensuring stability and performance, their applicability, their accompanying guarantees, and their merits and deficiencies are given. The possible extensions to less standard problem settings are also briefly discussed.}
}

Hanus, Raymond. "A new technique for preventing control windup." Journal A 21.1 (1980): 15-20.
@article{hanus1980new,
  title={A new technique for preventing control windup},
  author={Hanus, Raymond and others},
  journal={Journal A},
  volume={21},
  number={1},
  pages={15--20},
  year={1980}
}

Hanus, Raymond. "Contribution à la théorie des régulateurs conditionnés." (1978).
@article{hanus1978contribution,
  title={Contribution {\`a} la th{\'e}orie des r{\'e}gulateurs conditionn{\'e}s},
  author={Hanus, Raymond},
  year={1978},
  publisher={Universit{\'e} libre de Bruxelles}
}

R. Hanus, M. Kinnaert, J.-L. Henrotte, Conditioning technique, a general anti-windup and bumpless transfer method, Automatica, Volume 23, Issue 6, 1987, Pages 729-739, ISSN 0005-1098, https://doi.org/10.1016/0005-1098(87)90029-X.
@article{HANUS1987729,
  title = {Conditioning technique, a general anti-windup and bumpless transfer method},
  journal = {Automatica},
  volume = {23},
  number = {6},
  pages = {729-739},
  year = {1987},
  issn = {0005-1098},
  doi = {https://doi.org/10.1016/0005-1098(87)90029-X},
  url = {https://www.sciencedirect.com/science/article/pii/000510988790029X},
  author = {R. Hanus and M. Kinnaert and J.-L. Henrotte},
  keywords = {Nonlinear systems, saturation, multivariable systems, cascade control, anti-windup (not in the standard list), bumpless transfer (not in the standard list), initialization (not in the standard list)},
  abstract = {This paper gives a general way to take into account, by an appropriate design of the controller, any discrepancy which can occur between the actual inputs of a process and the desired outputs of its controller. This yields to the so-called conditioned control algorithms. The conditioning technique is described for multiple-input-multiple-output nonlinear controllers. Application to complex control structures is explained. The notion of a self-conditioned controller is defined in order to simplify the implementation of the conditioning technique. Some considerations about the stability of conditioned systems are given. The automatic initialization of any control algorithm is given as an application of the method. It shows that bumpless transfer can be achieved.}
}

Hippe, Peter. Windup in Control Owing to Sensor Saturation. Berlin, Heidelberg, New York, London: Springer, 2021.
@book{hippe2021windup,
  title={Windup in Control Owing to Sensor Saturation},
  author={Hippe, Peter},
  year={2021},
  publisher={Springer}
}

Mayuresh V. Kothare, Peter J. Campo, Manfred Morari, Carl N. Nett, A unified framework for the study of anti-windup designs, Automatica, Volume 30, Issue 12, 1994, Pages 1869-1883, ISSN 0005-1098, https://doi.org/10.1016/0005-1098(94)90048-5.
@article{KOTHARE19941869,
  title = {A unified framework for the study of anti-windup designs},
  journal = {Automatica},
  volume = {30},
  number = {12},
  pages = {1869-1883},
  year = {1994},
  issn = {0005-1098},
  doi = {https://doi.org/10.1016/0005-1098(94)90048-5},
  url = {https://www.sciencedirect.com/science/article/pii/0005109894900485},
  author = {Mayuresh V. Kothare and Peter J. Campo and Manfred Morari and Carl N. Nett},
  keywords = {Control nonlinearities, feedback control, nonlinear systems, multivariable control systems, PID control, saturation, saturation control, anti-windup (not in the standard list), bumpless transfer (not in the standard list)},
  abstract = {We present a unified framework for the study of linear time-invariant (LTI) systems subject to control input nonlinearities. The framework is based on the following two-step design paradigm: ‘design the linear controller ignoring control input nonlinearities and then add anti-windup bumpless transfer (AWBT) compensation to minimize the adverse effects of any control input nonlinearities on closed loop performance’. The resulting AWBT compensation is applicable to multivariable controllers of arbitrary structure and order. All known LTI anti-windup and/or bumpless transfer compensation schemes are shown to be special cases of this framework. This unification of existing schemes for AWBT compensation under a general framework is the main result of the paper.}
}

Youbin Peng, D. Vrancic and R. Hanus, "Anti-windup, bumpless, and conditioned transfer techniques for PID controllers," in IEEE Control Systems Magazine, vol. 16, no. 4, pp. 48-57, Aug. 1996, doi: 10.1109/37.526915.
@ARTICLE{526915,
  author={Youbin Peng and Vrancic, D. and Hanus, R.},
  journal={IEEE Control Systems Magazine},
  title={Anti-windup, bumpless, and conditioned transfer techniques for PID controllers},
  year={1996},
  volume={16},
  number={4},
  pages={48-57},
  abstract={Gives a simple and comprehensive review of anti-windup, bumpless and conditioned transfer techniques in the framework of the PID controller. We show that the most suitable anti-windup strategy for usual applications is the conditioning technique, using the notion of the realizable reference. The exception is the case in which the input limitations are too restrictive. In this case, we propose the anti-windup method with a free parameter tuned to obtain a compromise between the incremental algorithm and the conditioning technique. We also introduce the new notion of conditioned transfer, and we it to be a more suitable solution than bumpless transfer. All the discussions are supported by simulations.},
  keywords={Closed loop systems;Three-term control;Automatic control;Windup;Switches;Actuators;Valves;Control design;Feedback;Control engineering},
  doi={10.1109/37.526915},
  ISSN={1941-000X},
  month={Aug}
}

Tarbouriech, Sophie, and Matthew Turner. "Anti-windup design: an overview of some recent advances and open problems." IET control theory & applications 3.1 (2009): 1-19.
@article{doi:10.1049/iet-cta:20070435,
  author = {S. Tarbouriech  and M. Turner },
  title = {Anti-windup design: an overview of some recent advances and open problems},
  journal = {IET Control Theory & Applications},
  volume = {3},
  issue = {1},
  pages = {1-19},
  year = {2009},
  doi = {10.1049/iet-cta:20070435},
  URL = {https://digital-library.theiet.org/doi/abs/10.1049/iet-cta%3A20070435},
  eprint = {https://digital-library.theiet.org/doi/pdf/10.1049/iet-cta%3A20070435},
  abstract = {The anti-windup technique which can be used to tackle the problems of stability and performance degradation for linear systems with saturated inputs is dealt with. The anti-windup techniques which can be found in the literature today have evolved from many sources and, even now, are diverse and somewhat disconnected from one another. In this survey, an overview of many recent anti-windup techniques is provided and their connections with each other are stated. The anti-windup technique is also explained within the context of its historical emergence and the likely future directions of the field are speculated. The focus is on so-called ‘modern’ anti-windup techniques which began to emerge during the end of the 20th century and which allow a priori guarantees on stability to be made. The survey attempts to provide constructive LMI conditions for the synthesis of anti-windup compensators in both global and local contexts. Finally, some interesting extensions and open problems are discussed, such as nested saturations, the presence of time delays in the state or the input, and anti-windup for non-linear systems. }
}

## bibliografia non visitata
J. W. Choi and S. C. Lee, “Antiwindup strategy for PI-type speed controller,” IEEE Trans. Ind. Electron., vol. 56, no. 6, pp. 2039–2046, Jun. 2009.
J. K. Seok, “Frequency-spectrum-based antiwindup compensator for PI controlled systems,” IEEE Trans. Ind. Electron., vol. 53, no. 6, pp. 1781–1790, Dec. 2006.
K. S. Walgama, S. Ronnback, and J. Sternby, “Generalization of conditioning technique for anti-windup compensators,” Proc. Inst. Elect. Eng., vol. 139, pt. D, no. 2, pp. 109–118, Mar. 1992.
A. S. Hodel and C. E. Hall, “Variable-structure PID control to prevent integrator windup,” IEEE Trans. Ind. Electron., vol. 48, no. 2, pp. 442–451, Apr. 2001.
H. B. Shin, “New antiwindup PI controller for variable-speed motor drives,” IEEE Trans. Ind. Electron., vol. 45, no. 3, pp. 445–450, Jun. 1998.
J. G. Park, J. H. Chung, and H. B. Shin, “Anti-windup integral-proportional controller for variable-speed motor drives,” J. Power Electron., vol. 2, no. 2, pp. 130–138, Apr. 2002.
Fertik, H. A., & Ross, C. W. (1967). Direct digital control algorithm with anti-windup feature. ISA Transactions, 6(4), 317-328.
Astrom, K. J., & Hagglund, T. (2006). Advanced PID Control. ISA - Instrumentation, Systems, and Automation Society. (In particolare il Capitolo 3: Integrator Windup).
