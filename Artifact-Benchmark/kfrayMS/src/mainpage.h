/*! \mainpage Documentation de KF-Ray, raytracer parallèle
 *
 * \authors Karin Aït-Si-Amer & Florian Dang
 * \section Introduction
 * KF-Ray est un programme de rendu d'image 3D utilisant la méthode du lancer de rayons (raytracing).
 * KF-Ray peut être parallélisé.
 *
 * \section sytes Liens
 * Site de développement de KF-Ray :<br>
 * http://kf-ray.googlecode.com <br>
 *
 * Dernières releases disponibles : <br>
 * http://code.google.com/p/kf-ray/downloads/list
 *
 * Version publique de SVN (version en développement) : <br>
 * svn checkout http://kf-ray.googlecode.com/svn/trunk/ kf-ray-read-only
 *
 * Report de bugs et problèmes : <br>
 * http://code.google.com/p/kf-ray/issues/list
 *
 * Wiki : <br>
 * http://code.google.com/p/kf-ray/wiki/Welcome
 *
 * Site français de KF-Ray : <br>
 * http://kfray.free.fr
 *
 *
 * \section install_sec Installation
 *
 * Voir le fichier INSTALL
 *
 * \subsection tools_subsec Requis
 *
 * Base :
 * - GCC et Make
 * - ImageMagick (optionnel pour le display)
 *
 * Parser (optionnel) :
 * - Lex/Yacc
 *
 * Frontend GUI :
 * - GTK+ 2.x
 *
 * Parallélisation (optionnel) :
 * - MPI (OpenMPI et LAM/MPI par exemple)
 *
 * \subsection running Instructions
 * sh autogen.sh <br>
 * make <br>
 * cd src <br>
 * ./kfray [-i Scene] [-o Image] [-b Modele] [-t] [-a] [-l Lignes] [-d] <br>
 * Si vous voulez utiliser MPI, faites un ./configure CXX=mpicc CC=mpicc<br>
 *
 * Lisez le README ou le manuel d'utilisation pour plus d'informations.
 * <br>
 *
 * \section Contact
 * aitsiame [at] polytech.upmc.fr <br>
 * tdang [at] polytech.upmc.fr <br>
 * <i>Polytech'Paris UPMC 2009</i>
 *
 */
